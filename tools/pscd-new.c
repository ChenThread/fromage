#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <ctype.h>

#define BSWAP16(v) ((((v)>>8)&0xFF)|(((v)<<8)&0xFF00))
#define BSWAP32(v) (((BSWAP16(v)>>16)&0xFF)|((BSWAP16(v)<<16)&0xFF00))
#define TOLE16(v) (v)
#define TOBE16(v) BSWAP16(v)
#define TOLE32(v) (v)
#define TOBE32(v) BSWAP32(v)

/*
from nocash's psx-spx ( http://problemkaputt.de/psx-spx.htm ):

  00h 1      Length of Directory Record (LEN_DR) (33+LEN_FI+pad+LEN_SU)
  01h 1      Extended Attribute Record Length (usually 00h)
  02h 8      Data Logical Block Number (2x32bit)
  0Ah 8      Data Size in Bytes        (2x32bit)
  12h 7      Recording Timestamp       (yy-1900,mm,dd,hh,mm,ss,timezone)
  19h 1      File Flags 8 bits         (usually 00h=File, or 02h=Directory)
  1Ah 1      File Unit Size            (usually 00h)
  1Bh 1      Interleave Gap Size       (usually 00h)
  1Ch 4      Volume Sequence Number    (2x16bit, usually 0001h)
  20h 1      Length of Name            (LEN_FI)
  21h LEN_FI File/Directory Name ("FILENAME.EXT;1" or "DIR_NAME" or 00h or 01h)
  xxh 0..1   Padding Field (00h) (only if LEN_FI is even)
  xxh LEN_SU System Use (LEN_SU bytes) (see below for CD-XA disks)

LEN_SU can be calculated as "LEN_DR-(33+LEN_FI+Padding)". For CD-XA disks (as used in the PSX), LEN_SU is 14 bytes:

  00h 2      Owner ID Group  (whatever, usually 0000h, big endian)
  02h 2      Owner ID User   (whatever, usually 0000h, big endian)
  04h 2      File Attributes (big endian):
	       0   Owner Read    (usually 1)
	       1   Reserved      (0)
	       2   Owner Execute (usually 1)
	       3   Reserved      (0)
	       4   Group Read    (usually 1)
	       5   Reserved      (0)
	       6   Group Execute (usually 1)
	       7   Reserved      (0)
	       8   World Read    (usually 1)
	       9   Reserved      (0)
	       10  World Execute (usually 1)
	       11  IS_MODE2        (0=MODE1 or CD-DA, 1=MODE2)
	       12  IS_MODE2_FORM2  (0=FORM1, 1=FORM2)
	       13  IS_INTERLEAVED  (0=No, 1=Yes...?) (by file and/or channel?)
	       14  IS_CDDA         (0=Data or ADPCM, 1=CD-DA Audio Track)
	       15  IS_DIRECTORY    (0=File or CD-DA, 1=Directory Record)
	     Commonly used Attributes are:
	       0D55h=Normal Binary File (with 800h-byte sectors)
	       2555h=Unknown            (wipeout .AV files) (MODE1 ??)
	       4555h=CD-DA Audio Track  (wipeout .SWP files, alone .WAV file)
	       3D55h=Streaming File     (ADPCM and/or MDEC or so)
	       8D55h=Directory Record   (parent-, current-, or sub-directory)
  06h 2      Signature     ("XA")
  08h 1      File Number   (Must match Subheader's File Number)
  09h 5      Reserved      (00h-filled)
*/

typedef enum dentmode
{
	DENT_DAT, // Normal data
	DENT_RAW, // Raw sectors
	DENT_DIR, // Directory
} dentmode_t;

#define FNAME_MAX_LEN_ISO 32
#define FNAME_MAX_LEN_LOC 128
typedef struct isodent {
	uint8_t len_dr;
	uint8_t earl; // 0x00
	uint32_t dblk_le, dblk_be;
	uint32_t dlen_le, dlen_be;
	uint8_t ts_year; // year = ts_year+1900
	uint8_t ts_month;
	uint8_t ts_day;
	uint8_t ts_hour;
	uint8_t ts_minute;
	uint8_t ts_second;
	uint8_t ts_timezone;
	uint8_t flags; // 0x00 == file, 0x02 == directory
	uint8_t unitsz; // 0x00
	uint8_t ilgapsz; // 0x00
	uint16_t vsnum_le, vsnum_be; // 0x0001
	uint8_t len_fi;
	char fname[FNAME_MAX_LEN_ISO];
} __attribute__((__packed__)) isodent_t;

typedef struct xadent {
	uint16_t gid; // big-endian, usually 0x0000
	uint16_t uid; // big-endian, usually 0x0000
	uint16_t fattr; // big-endian
	// usual fattr vals (BE/LE):
	// * 0x0D55/0x550D normal file
	// * 0x3D55/0x553D XA/STR m2f2 file
	// * 0x8D55/0x558D directory
	uint8_t magic[2]; // "XA"
	uint8_t filenum; // equal to the sector subheader file number
	uint8_t reserved1[5]; // all 0x00
} __attribute__((__packed__)) xadent_t;

typedef struct locdent {
	isodent_t isodent;
	xadent_t xadent;
	char loc_fname[FNAME_MAX_LEN_LOC];
	char dir_fname[FNAME_MAX_LEN_LOC];
	dentmode_t dmode;
	int path_idx; // -1 == not a dir
	int parent_dir; // -1 == no parent
	int parent_path; // -1 == no parent
	int sector;
} locdent_t;

/*
Primary Volume Descriptor (sector 16 on PSX disks)

  000h 1    Volume Descriptor Type        (01h=Primary Volume Descriptor)
  001h 5    Standard Identifier           ("CD001")
  006h 1    Volume Descriptor Version     (01h=Standard)
  007h 1    Reserved                      (00h)
  008h 32   System Identifier             (a-characters) ("PLAYSTATION")
  028h 32   Volume Identifier             (d-characters) (max 8 chars for PSX?)
  048h 8    Reserved                      (00h)
  050h 8    Volume Space Size             (2x32bit, number of logical blocks)
  058h 32   Reserved                      (00h)
  078h 4    Volume Set Size               (2x16bit) (usually 0001h)
  07Ch 4    Volume Sequence Number        (2x16bit) (usually 0001h)
  080h 4    Logical Block Size in Bytes   (2x16bit) (usually 0800h) (1 sector)
  084h 8    Path Table Size in Bytes      (2x32bit) (max 800h for PSX)
  08Ch 4    Path Table 1 Block Number     (32bit little-endian)
  090h 4    Path Table 2 Block Number     (32bit little-endian) (or 0=None)
  094h 4    Path Table 3 Block Number     (32bit big-endian)
  098h 4    Path Table 4 Block Number     (32bit big-endian) (or 0=None)
  09Ch 34   Root Directory Record         (see next chapter)
  0BEh 128  Volume Set Identifier         (d-characters) (usually empty)
  13Eh 128  Publisher Identifier          (a-characters) (company name)
  1BEh 128  Data Preparer Identifier      (a-characters) (empty or other)
  23Eh 128  Application Identifier        (a-characters) ("PLAYSTATION")
  2BEh 37   Copyright Filename            ("FILENAME.EXT;VER") (empty or text)
  2E3h 37   Abstract Filename             ("FILENAME.EXT;VER") (empty)
  308h 37   Bibliographic Filename        ("FILENAME.EXT;VER") (empty)
  32Dh 17   Volume Creation Timestamp     ("YYYYMMDDHHMMSSFF",timezone)
  33Eh 17   Volume Modification Timestamp ("0000000000000000",00h)
  34Fh 17   Volume Expiration Timestamp   ("0000000000000000",00h)
  360h 17   Volume Effective Timestamp    ("0000000000000000",00h)
  371h 1    File Structure Version        (01h=Standard)
  372h 1    Reserved for future           (00h-filled)
  373h 141  Application Use Area          (00h-filled for PSX)
  400h 8    CD-XA Identifying Signature   ("CD-XA001" for PSX)
  408h 2    CD-XA Flags (unknown purpose) (00h-filled for PSX)
  40Ah 8    CD-XA Startup Directory       (00h-filled for PSX)
  412h 8    CD-XA Reserved                (00h-filled for PSX)
  41Ah 345  Application Use Area          (00h-filled for PSX)
  573h 653  Reserved for future           (00h-filled)

Volume Descriptor Set Terminator (sector 17 on PSX disks)

  000h 1    Volume Descriptor Type    (FFh=Terminator)
  001h 5    Standard Identifier       ("CD001")
  006h 1    Terminator Version        (01h=Standard)
  007h 2041 Reserved                  (00h-filled)
*/

typedef struct pvd {
	uint8_t vdtype; // 0x01 = PVD
	uint8_t magic1[5]; // "CD001"
	uint8_t vdver; // 0x01
	uint8_t reserved2[1]; // 0x00
	uint8_t iden_sys[32]; // "PLAYSTATION"
	uint8_t iden_vol[32]; // max 8 chars for some reason
	uint8_t reserved4[8];
	uint32_t lbn_count_le, lbn_count_be;
	uint8_t reserved3[32];
	uint16_t vset_size_le, vset_size_be; // 0x0001
	uint16_t vset_seqn_le, vset_seqn_be; // 0x0001
	uint16_t block_size_le, block_size_be; // 0x0800
	uint32_t ptsize_size_le, ptsize_size_be; // max 0x0800
	uint32_t ptent1_le;
	uint32_t ptent2_le; // often 0
	uint32_t ptent3_be;
	uint32_t ptent4_be; // often 0
	uint8_t rootdir_record[34];
	uint8_t iden_volset[128]; // ""
	uint8_t iden_publisher[128]; // "company name"
	uint8_t iden_datprep[128]; // ""
	uint8_t iden_app[128]; // "PLAYSTATION"
	uint8_t fname_copyright[37]; // ""
	uint8_t fname_abstract[37]; // ""
	uint8_t fname_biblio[37]; // ""
	uint8_t ts_creation[37]; // "YYYYMMDDHHMMSSFF" + timezone
	uint8_t ts_modification[37]; // "0000000000000000" + 0x00
	uint8_t ts_expiration[37]; // "0000000000000000" + 0x00
	uint8_t ts_effective[37]; // "0000000000000000" + 0x00
	uint8_t fsver; // 0x01
	uint8_t reserved1[1]; // 0x00
	uint8_t appuse1[141]; // 0x00
	uint8_t xamagic1[8]; // "CD-XA001"
	uint8_t xapad1[18]; // 0x00
	uint8_t appuse2[345]; // 0x00
	uint8_t pad1[653];
}__attribute__((__packed__))  pvd_t;

typedef struct vdst {
	uint8_t vdtype; // 0xFF = Terminator
	uint8_t magic1[5]; // "CD001"
	uint8_t vdver; // 0x01
	uint8_t pad1[2041];
} __attribute__((__packed__)) vdst_t;

#define MAX_DENTS 2048
locdent_t dent_list[MAX_DENTS];
int dent_count = 0;
int dent_path_count = 0;
uint32_t sector_count = 0;

typedef enum secmode
{
	SEC_EMPTY = 0,
	SEC_AUDIO, // Raw
	SEC_MODE1, // Normal data
	SEC_MODE2_FORM1, // XA data
	SEC_MODE2_FORM2, // XA ADPCM

	SEC_RAW, // Provide raw sector and fix it up
} secmode_t;

void free_whole_file(char **bufptr, size_t *buf_len)
{
	if(*bufptr != NULL) {
		free(*bufptr);
	}
	*bufptr = NULL;
	*buf_len = 0;
}

int load_whole_file(char **bufptr, size_t *buf_len, const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	assert(fp != NULL);

	free_whole_file(bufptr, buf_len);
	*buf_len = 0;
	for(;;) {
		size_t step_size = (1<<20); // 1MB at a time
		*bufptr = realloc(*bufptr, *buf_len+step_size);
		size_t amt = fread(*bufptr + *buf_len, 1, step_size, fp);
		if(amt == 0) {
			assert(feof(fp));
			break;
		}
		*buf_len += amt;
	}
	*bufptr = realloc(*bufptr, *buf_len);

	fclose(fp);

	return 0;
}

uint32_t edc_table[256];
int GF8_LOG[256];
int GF8_ILOG[256];
int GF8_PRODUCT[43][256];

int subfunc(int a, int b)
{
	if(a>0)
	{
		a=GF8_LOG[a]-b;
		if(a<0)
			a += 255;

		a=GF8_ILOG[a];
	}

	return(a);
}

void init_tables(void)
{
	int i, j;

	// standard "fast-CRC" LUT except with a different polynomial
	for(i=0; i <= 0xFF; i++)
	{
		uint32_t x = i;
		for(j = 0; j <= 7; j++)
		{
			uint32_t carry = x&1;
			x >>= 1;

			if(carry)
				x ^= 0xD8018001;
		}

		edc_table[i]=x;
	}

	GF8_LOG[0x00]=0x00;
	GF8_ILOG[0xFF]=0x00;
	int x=0x01;
	for(i=0x00; i <= 0xFE; i++)
	{
		GF8_LOG[x]=i;
		GF8_ILOG[i]=x;

		int carry8bit = x&0x80;
		x <<= 1;
		if(carry8bit)
			x ^= 0x1D;

		x &= 0xFF;
	}

	for(j=0; j <= 42; j++)
	{
		int xx = GF8_ILOG[44-j];
		int yy = subfunc(xx ^ 1,0x19);

		xx = subfunc(xx,0x01);
		xx = subfunc(xx ^ 1,0x18);
		xx = GF8_LOG[xx];
		yy = GF8_LOG[yy];
		GF8_PRODUCT[j][0]=0x0000;
		for(i=0x01; i <= 0xFF; i++)
		{
			int x=xx+GF8_LOG[i];
			int y=yy+GF8_LOG[i];

			if(x>=255) x -= 255;
			if(y>=255) y -= 255;

			GF8_PRODUCT[j][i]=GF8_ILOG[x]+(GF8_ILOG[y] << 8);
		}
	}
}

void calc_parity(uint8_t *sector, int offs, int len, int j0, int step1, int step2)
{
	int i, j;

	int src=0x00c;
	int dst=0x81c+offs;
	int srcmax=dst;

	for(i = 0; i <= len-1; i++)
	{
		int base=src, x=0x0000, y=0x0000;
		for(j=j0; j <= 42; j++)
		{
			x ^= GF8_PRODUCT[j][sector[src+0]];
			y ^= GF8_PRODUCT[j][sector[src+1]];
			src += step1;
			if((step1 == 2*44) && (src>=srcmax))
				src -= 2*1118;
		}

		sector[dst+2*len+0]=x & 0x0FF; sector[dst+0]=x >> 8;
		sector[dst+2*len+1]=y & 0x0FF; sector[dst+1]=y >> 8;
		dst += 2;
		src = base + step2;
	}
}

void calc_p_parity(uint8_t *sector)
{
	calc_parity(sector,0,43,19,2*43,2);
}

void calc_q_parity(uint8_t *sector)
{
	calc_parity(sector,43*4,26,0,2*44,2*43);
}

void adjust_edc(uint8_t *addr, int len)
{
	int i;
	uint32_t x=0x00000000;

	for(i=0; i <= len-1; i++)
	{
		x ^= (uint32_t)(uint8_t)addr[i];
		x = (x>>8) ^ edc_table[x & 0xFF];
	}

	//append EDC value (little endian)
	addr[0*4+len+0] = x >> 0;
	addr[0*4+len+1] = x >> 8;
	addr[0*4+len+2] = x >> 16;
	addr[0*4+len+3] = x >> 24;
}

int tobcd8(int v)
{
	return (v%10)+((v/10)<<4);
}

void encode_sector(uint8_t *rawsec, const uint8_t *srcsec, int lba, secmode_t secmode)
{
	// Sync
	memset(rawsec+0x000, 0x00, 0x930-0x000);
	memset(rawsec+0x001, 0xFF, 0x00B-0x001);

	// Time
	lba += 75*2;
	rawsec[0x00C] = tobcd8((lba/75)/60);
	rawsec[0x00D] = tobcd8((lba/75)%60);
	rawsec[0x00E] = tobcd8(lba%75);

	//bool is_raw = (secmode == SEC_RAW);

	if(secmode == SEC_RAW) {
		// Adjust it so it works
		memcpy(rawsec+0x010, srcsec+0x010, 0x930-0x010);
		if(srcsec[0x00F] == 0) {
			secmode = SEC_EMPTY;

		} else if(srcsec[0x00F] == 1) {
			secmode = SEC_MODE1;
			srcsec += 0x010;

		} else if(srcsec[0x00F] == 2) {
			if((srcsec[0x012] & 0x20) == 0) {
				secmode = SEC_MODE2_FORM1;
				srcsec += 0x018;
			} else {
				secmode = SEC_MODE2_FORM2;
			}
		} else {
			assert(!"invalid raw sector type");
			abort();
		}
	}

	switch(secmode)
	{
		case SEC_EMPTY:
			rawsec[0x00F] = 0x00;
			break;

		case SEC_MODE1:
			rawsec[0x00F] = 0x01;
			memcpy(rawsec+0x010, srcsec, 0x800);
			//*(uint32_t *)(&rawsec[0x810]) = calculate_edc(rawsec, 0x000, 0x810);
			adjust_edc(rawsec+0x000, 0x810);
			calc_p_parity(rawsec);
			calc_q_parity(rawsec);
			break;

		case SEC_MODE2_FORM1:
			rawsec[0x00F] = 0x02;
			rawsec[0x010] = rawsec[0x014] = 0x00;
			rawsec[0x011] = rawsec[0x015] = 0x00;
			rawsec[0x012] = rawsec[0x016] = 0x08;
			rawsec[0x013] = rawsec[0x017] = 0x00;
			memcpy(rawsec+0x018, srcsec, 0x800);
			adjust_edc(rawsec+0x010, 0x808);
			{
				uint32_t bakhed = *(uint32_t *)(&rawsec[0x00C]);
				calc_p_parity(rawsec);
				calc_q_parity(rawsec);
				*(uint32_t *)(&rawsec[0x00C]) = bakhed;
			}
			break;

		case SEC_MODE2_FORM2:
			// Use raw sectors and patch the result
			memcpy(rawsec+0x010, srcsec+0x010, 0x930-0x010);
			rawsec[0x00F] = 0x02;
			rawsec[0x010] = rawsec[0x014];
			rawsec[0x011] = rawsec[0x015];
			rawsec[0x012] = (rawsec[0x016] |= 0x20);
			rawsec[0x013] = rawsec[0x017];
			if(*(uint32_t *)(&rawsec[0x92C]) != 0x00000000) {
				adjust_edc(rawsec+0x010, 0x91C);
			}
			break;

		default:
			assert(!"rip");
			abort();
			break;
	}
}

int find_dent(const char *fname)
{
	for(int i = 0; i < dent_count; i++) {
		if(!strcmp(dent_list[i].loc_fname, fname)) {
			return i;
		}
	}

	return -1;
}

int assign_dent(const char *fname_in, dentmode_t dmode)
{
	// See if we need to split the string
	char fname_buf[FNAME_MAX_LEN_LOC];
	strncpy(fname_buf, fname_in, sizeof(fname_buf));
	char *c_sep = strchr(fname_buf, '/');
	int parent_idx = -1;
	if(c_sep != NULL) {
		for(;;) {
			char *newsep = strchr(c_sep+1, '/');
			if(newsep == NULL) {
				break;
			}
			c_sep = newsep;
		}
		*c_sep = '\x00';
		parent_idx = assign_dent(fname_buf, DENT_DIR);
	} else {
		c_sep = fname_buf-1;
	}

	// See if we have it already
	int ent_idx = find_dent(fname_in);
	if(ent_idx != -1) {
		return ent_idx;
	}

	// Allocate an entry
	ent_idx = (dent_count++);
	locdent_t *D = &dent_list[ent_idx];

	if(ent_idx == 0) { // assuming '.'
		parent_idx = ent_idx;
	}

	// Fill it in
	strncpy(D->loc_fname, fname_in, sizeof(D->loc_fname));
	strncpy(D->isodent.fname, c_sep+1, sizeof(D->isodent.fname));
	for(int i = 0; D->isodent.fname[i] != '\x00'; i++) {
		D->isodent.fname[i] = toupper(D->isodent.fname[i]);
	}
	strncpy(D->dir_fname, D->isodent.fname, sizeof(D->dir_fname));
	if(dmode != DENT_DIR) {
		strncat(D->isodent.fname, ";1", sizeof(D->isodent.fname));
	}
	D->dmode = dmode;
	D->path_idx = (dmode != DENT_DIR ? -1 : dent_path_count++);
	D->sector = (dmode != DENT_DIR ? 0 : D->path_idx + 22);
	D->parent_dir = parent_idx;
	D->parent_path = (parent_idx == -1 ? -1 : dent_list[parent_idx].path_idx);
	printf("%d %d %d %d \"%s\" \"%s\"\n", ent_idx, dmode, parent_idx, D->parent_path, fname_in, c_sep+1);

	D->isodent.len_fi = strlen(D->isodent.fname);
	D->isodent.len_dr = 33+14+((D->isodent.len_fi+1)&~1);
	D->isodent.flags = (dmode == DENT_DIR ? 0x02 : 0x00);
	D->isodent.vsnum_le = TOLE16(0x0001);
	D->isodent.vsnum_be = TOBE16(0x0001);
	D->xadent.fattr = (dmode == DENT_RAW ? TOBE16(0x3D55)
		: dmode == DENT_DAT ? TOBE16(0x0D55)
		: TOBE16(0x8D55));
	D->xadent.magic[0] = 'X';
	D->xadent.magic[1] = 'A';
	D->xadent.filenum = 0x00; // TODO: make use of this

	// Return!
	return ent_idx;
}

int main(int argc, char *argv[])
{
	init_tables();

	if(argc <= 1) {
		printf("usage:\n\t%s manifest.txt\n", argv[0]);
	}

	//
	// Parse manifest
	//

	FILE *manifestfp = fopen(argv[1], "r");
#define LINEBUF_MAX 1024
	char linebuf[LINEBUF_MAX];

	char *fname_bin = NULL;
	char *fname_cue = NULL;
	char *fname_lic = NULL;

	//assign_dent(".", DENT_DIR);

	for(;;)
	{
		// Get line
		char *ret_fgets = fgets(linebuf, sizeof(linebuf), manifestfp);
		if(ret_fgets == NULL) {
			assert(feof(manifestfp));
			break;
		}

		// Strip comment/newlines
		char *c_comnl = strpbrk(linebuf, "\r\n#");
		if(c_comnl != NULL) {
			*c_comnl = '\x00';
		}

		// Strip leading whitespace
		size_t wsbeg = strspn(linebuf, " \t\r\n");
		if(wsbeg != 0) {
			size_t l = strlen(linebuf);
			memmove(linebuf, linebuf+wsbeg, l-wsbeg+1);
		}

		// Skip empty lines
		if(linebuf[0] == '\x00') {
			continue;
		}

		// Do a split against '='
		char *c_eq = strchr(linebuf, '=');
		if(c_eq == NULL) {
			printf("ERROR: expected '=' in line: \"%s\"\n", linebuf);
			return 1;
		}
		assert(*c_eq == '=');
		*c_eq = '\x00';

		char *arg1 = c_eq+1;
		if(!strcmp(linebuf, "bin")) {
			assert(fname_bin == NULL);
			fname_bin = strdup(arg1);
		} else if(!strcmp(linebuf, "cue")) {
			assert(fname_cue == NULL);
			fname_cue = strdup(arg1);
		} else if(!strcmp(linebuf, "lic")) {
			assert(fname_lic == NULL);
			fname_lic = strdup(arg1);

		} else if(!strcmp(linebuf, "dat")) {
			assign_dent(arg1, DENT_DAT);
		} else if(!strcmp(linebuf, "raw")) {
			assign_dent(arg1, DENT_RAW);

		} else {
			printf("ERROR: unhandled: [%s] = [%s]\n", linebuf, arg1);
			return 1;
		}
	}

	// Close manifest
	fclose(manifestfp);

	// Ensure that we are ready to make an image
	assert(fname_bin != NULL);
	assert(fname_cue != NULL);
	assert(fname_lic != NULL);

	printf("Building CD image...\n");

	// Load licence file
	char *licence_buf = NULL;
	size_t licence_len = 0;
	load_whole_file(&licence_buf, &licence_len, fname_lic);
	assert(licence_len == 0x930*16);

	// Start producing bin file
	FILE *binfp = fopen(fname_bin, "w+b");
	uint8_t secdata_out[0x930];
	uint8_t secdata_in_data[0x800];
	uint8_t secdata_in_raw[0x930];
	assert(binfp != NULL);
	fwrite(licence_buf, licence_len, 1, binfp);
	sector_count = 22 + dent_path_count;

	// Generate path table
	// We have to do a little-endian ver and a big-endian ver
	int ptsize = 0;
	for(int i = 0; i < 4; i+=2) {
		ptsize = 0;
		fseek(binfp, 0x930*(18+i), SEEK_SET);
		memset(secdata_in_data, 0, sizeof(secdata_in_data));

		// XXX: Does this require everything to be ordered by depth?
		// If so, this will need a rework.

		for(int j = 0; j < dent_count; j++) {
			locdent_t *D = &dent_list[j];

			if(D->dmode != DENT_DIR) { continue; }

			/*
			00h 1       Length of Directory Name (LEN_DI) (01h..08h for PSX)
			01h 1       Extended Attribute Record Length  (usually 00h)
			02h 4       Directory Logical Block Number
			06h 2       Parent Directory Number           (0001h and up)
			08h LEN_DI  Directory Name (d-characters, d1-characters) (or 00h for Root)
			xxh 0..1    Padding Field (00h) (only if LEN_FI is odd)
			*/

			int len_di = strlen(D->dir_fname);
			//printf("%d %d \"%s\" \"%s\" %d\n" , D->path_idx, D->parent_path , D->loc_fname , D->dir_fname , len_di);
			secdata_in_data[ptsize++] = len_di;
			secdata_in_data[ptsize++] = 0x00;
			*(uint32_t *)(secdata_in_data+ptsize) = (
				i == 0
				? TOLE32(D->sector)
				: TOBE32(D->sector)
			); ptsize += 4;
			*(uint16_t *)(secdata_in_data+ptsize) = (
				i == 0
				? TOLE16(D->parent_dir+1)
				: TOBE16(D->parent_dir+1)
			); ptsize += 2;
			strncpy((char *)secdata_in_data+ptsize, D->dir_fname, len_di);
			if(D->dir_fname[0] == '.') {
				secdata_in_data[ptsize] = '\x00';
			}
			ptsize += (len_di+1)&~1;
			assert(ptsize <= 0x800);
		}

		encode_sector(secdata_out, secdata_in_data, (18+i), SEC_MODE2_FORM1);
		fwrite(secdata_out, 0x930, 1, binfp);
		encode_sector(secdata_out, secdata_in_data, (19+i), SEC_MODE2_FORM1);
		fwrite(secdata_out, 0x930, 1, binfp);
	}
	printf("ptsize = %d\n", ptsize);

	// Put files everywhere
	for(int i = 0; i < dent_count; i++) {
		locdent_t *D = &dent_list[i];
		//if(D->dmode == DENT_DIR) { continue; }
		printf("file \"%s\"\n", D->loc_fname);

		switch(D->dmode) {
			case DENT_DAT: {
				char *dat_buf = NULL;
				size_t dat_len = 0;
				load_whole_file(&dat_buf, &dat_len, D->loc_fname);
				int dat_sectors = (dat_len+0x7FF)/0x800;

				D->sector = sector_count;
				D->isodent.dblk_le = TOLE32(D->sector);
				D->isodent.dblk_be = TOBE32(D->sector);
				D->isodent.dlen_le = TOLE32(dat_len);
				D->isodent.dlen_be = TOBE32(dat_len);
				sector_count += dat_sectors;

				//fseek(binfp, 0x930*(D->sector+j), SEEK_SET);
				fseek(binfp, 0x930*(D->sector), SEEK_SET);
				for(int j = 0; j < dat_sectors; j++) {
					memset(secdata_in_data, 0, sizeof(secdata_in_data));
					memcpy(secdata_in_data, dat_buf+0x800*j,
						(j < dat_sectors-1 ? 0x800: dat_len-0x800*j));
					encode_sector(secdata_out, secdata_in_data, (D->sector+j), SEC_MODE2_FORM1);
					fwrite(secdata_out, 0x930, 1, binfp);
				}

				free_whole_file(&dat_buf, &dat_len);
			} break;

			case DENT_RAW: {
				char *raw_buf = NULL;
				size_t raw_len = 0;
				load_whole_file(&raw_buf, &raw_len, D->loc_fname);
				int raw_sectors = (raw_len+0x92F)/0x930;
				int raw_normlen = raw_sectors*0x800;

				D->sector = sector_count;
				D->isodent.dblk_le = TOLE32(D->sector);
				D->isodent.dblk_be = TOBE32(D->sector);
				D->isodent.dlen_le = TOLE32(raw_normlen);
				D->isodent.dlen_be = TOBE32(raw_normlen);
				sector_count += raw_sectors;

				fseek(binfp, 0x930*(D->sector), SEEK_SET);
				for(int j = 0; j < raw_sectors; j++) {
					memset(secdata_in_raw, 0, sizeof(secdata_in_raw));
					memcpy(secdata_in_raw, raw_buf+0x930*j,
						(j < raw_sectors-1 ? 0x930: raw_len-0x930*j));
					//fseek(binfp, 0x930*(D->sector+j), SEEK_SET);
					encode_sector(secdata_out, secdata_in_raw, (D->sector+j), SEC_RAW);
					fwrite(secdata_out, 0x930, 1, binfp);
				}

				free_whole_file(&raw_buf, &raw_len);
			} break;

			case DENT_DIR:
				// Do nothing
				D->isodent.dlen_le = TOLE32(0x800);
				D->isodent.dlen_be = TOBE32(0x800);
				break;

			default:
				assert(!"halp");
				abort();
				return 99;
		}
	}

	// Generate directories
	for(int i = 0; i < dent_count; i++) {
		locdent_t *D = &dent_list[i];
		if(D->dmode != DENT_DIR) { continue; }
		fseek(binfp, 0x930*(22+D->path_idx), SEEK_SET);

		memset(secdata_in_data, 0, sizeof(secdata_in_data));

		printf("Directory! %d %d %d \"%s\"\n", i, D->path_idx, D->sector, D->isodent.fname);
		uint8_t *p = secdata_in_data;

		// Generate main link
		memcpy(p, &D->isodent, sizeof(D->isodent)-FNAME_MAX_LEN_ISO);
		((isodent_t *)p)->len_fi = 1;
		((isodent_t *)p)->len_dr = 33+1+14;
		p += sizeof(D->isodent)-FNAME_MAX_LEN_ISO;
		*(p++) = '\x00';
		memcpy(p, &D->xadent, sizeof(D->xadent));
		p += sizeof(D->xadent);

		// Generate back link
		assert(D->parent_dir >= 0);
		locdent_t *E = &dent_list[D->parent_dir];
		memcpy(p, &E->isodent, sizeof(E->isodent)-FNAME_MAX_LEN_ISO);
		((isodent_t *)p)->len_fi = 1;
		((isodent_t *)p)->len_dr = 33+1+14;
		p += sizeof(E->isodent)-FNAME_MAX_LEN_ISO;
		*(p++) = '\x01';
		memcpy(p, &E->xadent, sizeof(E->xadent));
		p += sizeof(E->xadent);

		// Generate file stuff
		for(int j = 0; j < dent_count; j++) {
			locdent_t *F = &dent_list[j];
			if(j == i) { continue; }
			if(F->parent_dir != i) { continue; }
			printf("- %d %d \"%s\"\n", j, F->sector, F->isodent.fname);
			memcpy(p, &F->isodent, sizeof(F->isodent)-FNAME_MAX_LEN_ISO+F->isodent.len_fi);
			p += sizeof(F->isodent)-FNAME_MAX_LEN_ISO+((F->isodent.len_fi+1)&~1);
			memcpy(p, &F->xadent, sizeof(F->xadent));
			p += sizeof(F->xadent);
		}

		// TODO!

		encode_sector(secdata_out, secdata_in_data, (22+D->path_idx), SEC_MODE2_FORM1);
		fwrite(secdata_out, 0x930, 1, binfp);
	}

	printf("sector_count = %d\n", sector_count);

	// Generate PVD
	fseek(binfp, 0x930*16, SEEK_SET);
	pvd_t pvd = {
		.vdtype = 0x01, // 0x01 = PVD
		.magic1 = "CD001", // "CD001"
		.vdver = 0x01, // 0x01
		.iden_sys = "PLAYSTATION",
		.iden_vol = "", // max 8 chars for some reason
		.lbn_count_le = TOLE32(sector_count),
		.lbn_count_be = TOBE32(sector_count),
		.vset_size_le = TOLE16(0x0001),
		.vset_size_be = TOBE16(0x0001),
		.vset_seqn_le = TOLE16(0x0001),
		.vset_seqn_be = TOBE16(0x0001),
		.block_size_le = TOLE16(0x0800),
		.block_size_be = TOBE16(0x0800),
		.ptsize_size_le = TOLE16(ptsize),
		.ptsize_size_be = TOBE16(ptsize),
		.ptent1_le = TOLE32(18),
		.ptent2_le = TOLE32(19),
		.ptent3_be = TOBE32(20),
		.ptent4_be = TOBE32(21),
		.rootdir_record = {
			34, 0,
			22,0,0,0, 0,0,0,22,
			0x00,0x08,0,0, 0,0,0x08,0x00, // length. do we patch this?
			0,0,0,0,0,0,0,
			0x02,
			0,0,
			1,0,0,1,
			1,0x00,
		},
		.iden_volset = "",
		.iden_publisher = "",
		.iden_datprep = "CHEN THREAD PSCD TOOLS",
		.iden_app = "PLAYSTATION",
		.fname_copyright = "",
		.fname_abstract = "",
		.fname_biblio = "",
		.ts_creation = "0000000000000000\x00", // "YYYYMMDDHHMMSSFF" + timezone
		.ts_modification = "0000000000000000\x00",
		.ts_expiration = "0000000000000000\x00",
		.ts_effective = "0000000000000000\x00",
		.fsver = 0x01,
		.xamagic1 = "CD-XA001",
	};
	encode_sector(secdata_out, (uint8_t *)&pvd, 16, SEC_MODE2_FORM1);
	fwrite(secdata_out, 0x930, 1, binfp);

	// Close bin file
	fclose(binfp);

	// Do cue file
	{
		// Open the file
		FILE *cuefp = fopen(fname_cue, "wb");
		assert(cuefp != NULL);

		// Get the base filename
		char *fnbase = fname_bin;
		for(;;) {
			char *c_psep = strchr(fnbase, '/');
			if(c_psep == NULL) { break; }
			fnbase = c_psep+1;
		}

		// Dump a header
		fprintf(cuefp, "FILE \"%s\" BINARY\n", fnbase);
		fprintf(cuefp, " TRACK 01 MODE2/2352\n");
		fprintf(cuefp, " INDEX 01 00:00:00\n");

		// We are done here.
		fclose(cuefp);
	}

#if 0
	// All the sectors!
	FILE *srcfp = fopen(argv[5], "rb");
	for(int i = 0;  true ; i++) {
		printf("Sector: %d/%d\n", i, wad_scount);

		fseek(fp, 0x930*(wad_sector+i), SEEK_SET);
		int wad_count = fread(wad_secdata, 0x930, 1, fp);
		uint8_t srcbuf[0x800];
		memset(srcbuf, 0, sizeof(srcbuf));
		size_t bytes_read = fread(srcbuf, 1, sizeof(srcbuf), srcfp);
		printf("bytes read: %d\n", (int)bytes_read);
		if(bytes_read == 0) {
			break;
		}

		//encode_sector(secdata_out, wad_secdata+0x018, wad_sector, SEC_MODE2_FORM1);
		encode_sector(secdata_out, srcbuf, wad_sector+i, SEC_MODE2_FORM1);
		//for(int i = 0; i < 0x930; i++) {
		for(int i = 0; i < 0x030; i++) {
			printf(" %02X", wad_secdata[i]^secdata_out[i]);
			if((i&0xF)==0xF) { printf("\n"); }
		}

		fseek(fp, 0x930*(wad_sector+i), SEEK_SET);
		fwrite(secdata_out, 0x930, 1, fp);
	}
#endif

	return 0;
}

