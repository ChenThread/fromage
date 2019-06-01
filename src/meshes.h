#define FACE_N 0x0000
#define FACE_0 0x0080
#define FACE_P 0x0100

const mesh_data_t mesh_data_slab[] = {
	// -Z
	{FACE_N, FACE_N, FACE_N, 0x0F00, .face = 0,},
	{FACE_P, FACE_N, FACE_N, 0x0F0F, .face = 0,},
	{FACE_N, FACE_0, FACE_N, 0x0800, .face = 0,},
	{FACE_P, FACE_0, FACE_N, 0x080F, .face = 0,},

	// +Z
	{FACE_N, FACE_N, FACE_P, 0x0F0F, .face = 1,},
	{FACE_N, FACE_0, FACE_P, 0x080F, .face = 1,},
	{FACE_P, FACE_N, FACE_P, 0x0F00, .face = 1,},
	{FACE_P, FACE_0, FACE_P, 0x0800, .face = 1,},

	// -X
	{FACE_N, FACE_N, FACE_N, 0x0F0F, .face = 2,},
	{FACE_N, FACE_0, FACE_N, 0x080F, .face = 2,},
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 2,},
	{FACE_N, FACE_0, FACE_P, 0x0800, .face = 2,},

	// +X
	{FACE_P, FACE_N, FACE_N, 0x0F00, .face = 3,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 3,},
	{FACE_P, FACE_0, FACE_N, 0x0800, .face = 3,},
	{FACE_P, FACE_0, FACE_P, 0x080F, .face = 3,},

	// -Y
	{FACE_N, FACE_N, FACE_N, 0x0000, .face = 4,},
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 4,},
	{FACE_P, FACE_N, FACE_N, 0x000F, .face = 4,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 4,},

	// +Y
	{FACE_N, FACE_0, FACE_N, 0x0000, .face = 5,},
	{FACE_P, FACE_0, FACE_N, 0x000F, .face = 5,},
	{FACE_N, FACE_0, FACE_P, 0x0F00, .face = 5,},
	{FACE_P, FACE_0, FACE_P, 0x0F0F, .face = 5,},
};

const mesh_data_t mesh_data_block[] = {
	// -Z
	{FACE_N, FACE_N, FACE_N, 0x0F00, .face = 0,},
	{FACE_P, FACE_N, FACE_N, 0x0F0F, .face = 0,},
	{FACE_N, FACE_P, FACE_N, 0x0000, .face = 0,},
	{FACE_P, FACE_P, FACE_N, 0x000F, .face = 0,},

	// +Z
	{FACE_N, FACE_N, FACE_P, 0x0F0F, .face = 1,},
	{FACE_N, FACE_P, FACE_P, 0x000F, .face = 1,},
	{FACE_P, FACE_N, FACE_P, 0x0F00, .face = 1,},
	{FACE_P, FACE_P, FACE_P, 0x0000, .face = 1,},

	// -X
	{FACE_N, FACE_N, FACE_N, 0x0F0F, .face = 2,},
	{FACE_N, FACE_P, FACE_N, 0x000F, .face = 2,},
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 2,},
	{FACE_N, FACE_P, FACE_P, 0x0000, .face = 2,},

	// +X
	{FACE_P, FACE_N, FACE_N, 0x0F00, .face = 3,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 3,},
	{FACE_P, FACE_P, FACE_N, 0x0000, .face = 3,},
	{FACE_P, FACE_P, FACE_P, 0x000F, .face = 3,},

	// -Y
	{FACE_N, FACE_N, FACE_N, 0x0000, .face = 4,},
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 4,},
	{FACE_P, FACE_N, FACE_N, 0x000F, .face = 4,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 4,},

	// +Y
	{FACE_N, FACE_P, FACE_N, 0x0000, .face = 5,},
	{FACE_P, FACE_P, FACE_N, 0x000F, .face = 5,},
	{FACE_N, FACE_P, FACE_P, 0x0F00, .face = 5,},
	{FACE_P, FACE_P, FACE_P, 0x0F0F, .face = 5,},
};

const mesh_data_t mesh_data_plant[] = {
	// TODO
	// X-Z=0 front
	{FACE_N, FACE_N, FACE_N, 0x0F00, .face = 0,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 0,},
	{FACE_N, FACE_P, FACE_N, 0x0000, .face = 0,},
	{FACE_P, FACE_P, FACE_P, 0x000F, .face = 0,},

	// X-Z=0 back
	{FACE_P, FACE_P, FACE_P, 0x000F, .face = 1,},
	{FACE_N, FACE_P, FACE_N, 0x0000, .face = 1,},
	{FACE_P, FACE_N, FACE_P, 0x0F0F, .face = 1,},
	{FACE_N, FACE_N, FACE_N, 0x0F00, .face = 1,},

	// X+Z=0 front
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 2,},
	{FACE_P, FACE_N, FACE_N, 0x0F0F, .face = 2,},
	{FACE_N, FACE_P, FACE_P, 0x0000, .face = 2,},
	{FACE_P, FACE_P, FACE_N, 0x000F, .face = 2,},

	// X+Z=0 back
	{FACE_P, FACE_P, FACE_N, 0x000F, .face = 3,},
	{FACE_N, FACE_P, FACE_P, 0x0000, .face = 3,},
	{FACE_P, FACE_N, FACE_N, 0x0F0F, .face = 3,},
	{FACE_N, FACE_N, FACE_P, 0x0F00, .face = 3,},
};
