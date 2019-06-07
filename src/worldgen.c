#include "common.h"

#define ELEVATION_MUL 0.025
#define ROUGHNESS_MUL 0.05
#define DETAIL_MUL 0.15

#define fade(t) ((t)*(t)*(t)*((t)*((t)*6-15)+10))
#define lerp(t,a,b) ((a)+((t)*((b)-(a))))

static inline float grad(uint8_t hash, float x, float y, float z)
{
	switch(hash & 0xF)
	{
		case  0: return  x +  y;
		case  1: return -x +  y;
		case  2: return  x + -y;
		case  3: return -x + -y;
		case  4: return  x +  z;
		case  5: return -x +  z;
		case  6: return  x + -z;
		case  7: return -x + -z;
		case  8: return  y +  z;
		case  9: return -y +  z;
		case 10: return  y + -z;
		case 11: return -y + -z;
		case 12: return  y +  x;
		case 13: return -y +  z;
		case 14: return  y + -x;
		case 15: return -y + -z;
		default: while(1) {}; break;
	}
}

static float perlin(uint8_t *perm, float x, float y, float z)
{
	int xi = (int)x & 0xFF;
	int yi = (int)y & 0xFF;
	int zi = (int)z & 0xFF;
	float xf = x-(int)x;
	float yf = y-(int)y;
	float zf = z-(int)z;
	float u = fade(xf);
	float v = fade(yf);
	float w = fade(zf);

	int a  = perm[xi]+yi;
	int aa = perm[a&0xFF]+zi;
	int ab = perm[(a+1)&0xFF]+zi;
	int b  = perm[(xi+1)&0xFF]+yi;
	int ba = perm[b&0xFF]+zi;
	int bb = perm[(b+1)&0xFF]+zi;

	uint8_t aaa = perm[aa&0xFF];
	uint8_t aba = perm[ab&0xFF];
	uint8_t baa = perm[ba&0xFF];
	uint8_t bba = perm[bb&0xFF];
	uint8_t aab = perm[(aa+1)&0xFF];
	uint8_t abb = perm[(ab+1)&0xFF];
	uint8_t bab = perm[(ba+1)&0xFF];
	uint8_t bbb = perm[(bb+1)&0xFF];

	return lerp(w, lerp(v, lerp(u, grad(aaa, xf  , yf  , zf   ),
	                               grad(baa, xf-1, yf  , zf   )),
	                       lerp(u, grad(aba, xf  , yf-1, zf   ),
	                               grad(bba, xf-1, yf-1, zf   ))),
	               lerp(v, lerp(u, grad(aab, xf  , yf  , zf-1 ),
	                               grad(bab, xf-1, yf  , zf-1 )),
	                       lerp(u, grad(abb, xf  , yf-1, zf-1 ),
	                               grad(bbb, xf-1, yf-1, zf-1 ))));
}

#define SET(x,y,z,b) if (x>=0&&y>=0&&z>=0&&x<lx&&y<ly&&z<lz) map[(((y) * lz + (z)) * lx + (x))] = (b)
#define GET(x,y,z) ((x>=0&&y>=0&&z>=0&&x<lx&&y<ly&&z<lz) ? map[(((y) * lz + (z)) * lx + (x))] : 0)

void world_generate(uint8_t *map, int32_t lx, int32_t ly, int32_t lz, uint32_t seed, worldgen_stage_callback *wc, save_progress_callback *pc)
{
	if (wc != NULL) wc("Raising..");

	uint8_t heights[lx*lz];
	uint8_t perm[256];
	for (int i = 0; i < 256; i++) perm[i] = i;

	int imax;

	for (int i = 255; i > 0; i--)
	{
		uint8_t j = seed & 0xFF;
		uint8_t t = perm[j]; perm[j] = perm[i]; perm[i] = t;
		RAND(seed);
	}

	for (int z = 0; z < lz; z++)
	for (int x = 0; x < lx; x++)
	{
		if (pc != NULL) pc(z*lx+x,lz*lx);

		float elevation = perlin(perm, x * ELEVATION_MUL, z * ELEVATION_MUL, 0);
		float roughness = perlin(perm, x * ROUGHNESS_MUL, z * ROUGHNESS_MUL, 10000);
		float detail = perlin(perm, x * DETAIL_MUL, z * DETAIL_MUL, 20000) / 2.0;
		int height = (int) ((elevation + (roughness * detail)) * (ly/4) + (ly/2) + 0.5);
		if (height > ly-1) height = ly-1;
		else if (height < 1) height = 1;

		heights[z*lx + x] = height;
		SET(x,0,z,10);

		if (height > ly/2) {
			for (int y = 1; y < height-4; y++)
				map[(y * lz + z) * lx + x] = 1;
			for (int y = height-4; y < height-1; y++)
				map[(y * lz + z) * lx + x] = 3;
			map[((height - 1) * lz + z) * lx + x] = 2;
		} else if (height == ly/2) {
			for (int y = 1; y < height-3; y++)
				map[(y * lz + z) * lx + x] = 1;
			for (int y = height-3; y < height; y++)
				map[(y * lz + z) * lx + x] = 12;
		} else {
			for (int y = 1; y < height; y++)
				map[(y * lz + z) * lx + x] = 1;
			for (int y = height; y < ly/2; y++)
				map[((y) * lz + z) * lx + x] = 8;
		}
	}

	if (wc != NULL) wc("Soiling..");
	imax = lx*lz/64;
	for (int i = 0; i < imax; i++)
	{
		if (pc != NULL) pc(i,imax);

		int ore_type = 14;
		switch (seed & 0x7) {
			case 0: break; // gold
			case 1: case 2: case 3: ore_type += 1; break; // iron
			default: ore_type += 2; break; // coal
		}
		RAND(seed);
		int cx = seed % lx;
		int cz = (seed / lx) % lz;
		int cy = (seed / lx / lz) % (ly/2);
		RAND(seed);
		int cluster_xs = (seed & 1);
		int cluster_ys = ((seed >> 1) & 1);
		int cluster_zs = ((seed >> 2) & 1);
		int cluster_xe = ((seed >> 3) & 1);
		int cluster_ye = ((seed >> 4) & 1);
		int cluster_ze = ((seed >> 5) & 1);
		RAND(seed);
		for (int dx=cx-cluster_xs; dx<=cx+cluster_xe; dx++)
		for (int dy=cy-cluster_ys; dy<=cy+cluster_ye; dy++)
		for (int dz=cz-cluster_zs; dz<=cz+cluster_ze; dz++)
			if (GET(dx,dy,dz) == 1) SET(dx,dy,dz,ore_type);
	}

#if 0
	if (wc != NULL) wc("Carving..");
	imax = 10*(lx*lz)/(64*64);
	for (int i = 0; i < imax; i++)
	{
		if (pc != NULL) pc(i,imax);
		double tx = seed % lx;
		double tz = (seed / lx) % lz;
		double ty = (seed / lx / lz) % (ly*2/3);
		RAND(seed);
		double perlin_x = ((seed & 0xFFFF) / 256.0);
		double perlin_z = ((seed >> 16) / 256.0);
		double perlin_y = 0.0;
		RAND(seed);
		int perlin_length = (seed & 0x1F) + 8;
		for (int i = 0; i < perlin_length; i++) {
			int cx = (int) (tx + 0.5);
			int cy = (int) (ty + 0.5);
			int cz = (int) (tz + 0.5);
			int depth = 1;

			for (int dx=cx-depth; dx<=cx+depth; dx++)
			for (int dy=cy-depth; dy<=cy+depth; dy++)
			for (int dz=cz-depth; dz<=cz+depth; dz++)
				SET(dx,dy,dz,0);

			double noise_val_1 = perlin(perm, perlin_x, perlin_y, perlin_z) * 3.14159253565 * 2;
			double noise_val_2 = perlin(perm, perlin_x, perlin_y, perlin_z + 1000.0) * 3.14159253565 * 2;
			double noise_val_3 = perlin(perm, perlin_x, perlin_y, perlin_z + 2000.0) * 3.14159253565 * 2;
			tx += sin(noise_val_1)/1.25;
			ty += cos(noise_val_1)/1.25;

			ty += sin(noise_val_2)/1.25;
			tz += cos(noise_val_2)/1.25;

			tz += sin(noise_val_3)/1.25;
			tx += cos(noise_val_3)/1.25;

			perlin_y += 0.05;
		}
	}
#endif

	if (wc != NULL) wc("Growing..");
	imax = lx*lz/256;
	for (int i = 0; i < imax; i++)
	{
		if (pc != NULL) pc(i,imax);
		int tx = seed % lx;
		int tz = (seed / lx) % lz;
		RAND(seed);
		if (heights[tz*lx+tx] > ly/2 && heights[tz*lx+tx] < ly-5)
		{
			// plant tree
			int y_base = heights[tz*lx+tx];
			if (GET(tx,y_base,tz) != 0) continue;

			int log_height = 3 + (seed % 3);
			RAND(seed);

			for (int i = 0; i < log_height; i++)
				SET(tx,i+y_base,tz,17);

			SET(tx,y_base+log_height,tz,18);
			SET(tx,y_base+log_height+1,tz,18);

			for (int i = -2; i <= 1; i++) {
				int sb = (i+2)*8;
				SET(tx,y_base+log_height+i,tz-1,18);
				SET(tx-1,y_base+log_height+i,tz,18);
				SET(tx,y_base+log_height+i,tz+1,18);
				SET(tx+1,y_base+log_height+i,tz,18);

				if (i < 1 || (i == 1 && (seed & (1<<sb)))) SET(tx-1,y_base+log_height+i,tz-1,18);
				if (i < 1 || (i == 1 && (seed & (2<<sb)))) SET(tx-1,y_base+log_height+i,tz+1,18);
				if (i < 1 || (i == 1 && (seed & (4<<sb)))) SET(tx+1,y_base+log_height+i,tz-1,18);
				if (i < 1 || (i == 1 && (seed & (8<<sb)))) SET(tx+1,y_base+log_height+i,tz+1,18);

				if (i <= -1)
				{
					SET(tx,y_base+log_height+i,tz-2,18);
					SET(tx-2,y_base+log_height+i,tz,18);
					SET(tx,y_base+log_height+i,tz+2,18);
					SET(tx+2,y_base+log_height+i,tz,18);

					SET(tx-2,y_base+log_height+i,tz-1,18);
					SET(tx-1,y_base+log_height+i,tz-2,18);
					SET(tx-2,y_base+log_height+i,tz+1,18);
					SET(tx-1,y_base+log_height+i,tz+2,18);
					SET(tx+2,y_base+log_height+i,tz-1,18);
					SET(tx+1,y_base+log_height+i,tz-2,18);
					SET(tx+2,y_base+log_height+i,tz+1,18);
					SET(tx+1,y_base+log_height+i,tz+2,18);

					if (i < -1 || ((seed & (1<<sb)))) SET(tx-2,y_base+log_height+i,tz-2,18);
					if (i < -1 || ((seed & (2<<sb)))) SET(tx-2,y_base+log_height+i,tz+2,18);
					if (i < -1 || ((seed & (4<<sb)))) SET(tx+2,y_base+log_height+i,tz-2,18);
					if (i < -1 || ((seed & (8<<sb)))) SET(tx+2,y_base+log_height+i,tz+2,18);
				}

				RAND(seed);
			}

			SET(tx-1,y_base+log_height,tz,18);
			SET(tx-1,y_base+log_height,tz-1,18);
		}
	}

	if (wc != NULL) wc("Planting..");
	imax = lx*lz/128;
	for (int i = 0; i < imax; i++)
	{
		if (pc != NULL) pc(i,imax);
		int tx = seed % lx;
		int tz = (seed / lx) % lz;
		RAND(seed);

		if (heights[tz*lx+tx] > ly/2)
		{
			// plant plant
			int y_base = heights[tz*lx+tx];
			if (GET(tx,y_base,tz) != 0) continue;

			SET(tx, y_base, tz, (seed&1) + 37);
			RAND(seed);
		}
	}

	if (pc != NULL) pc(1,1);
}
