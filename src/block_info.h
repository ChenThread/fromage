#define QUAD(x,y) {((y)<<12)|((x)<<4), 0x001C, 0x4034 | ( ((y) * 16 + (x)) << 6), 0x0000, 0x00FF00FF}

#define INFO_BLOCK_CUBE(x,y) {QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(x,y)}
#define INFO_BLOCK_CUBE_TB(x,y,xt,yt,xb,yb) {QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(xb,yb),QUAD(xt,yt)}
#define INFO_BLOCK_PLANT(x,y) {QUAD(x,y),QUAD(x,y),QUAD(x,y),QUAD(x,y)}
#define INFO_BLOCK_WOOL(i) INFO_BLOCK_CUBE(i,4)

block_info_t block_info[BLOCK_MAX][QUAD_MAX] = {
	{}, // 0: Air
	INFO_BLOCK_CUBE(1, 0), // 1: Stone
	INFO_BLOCK_CUBE_TB(3, 0, 0, 0, 2, 0), // 2: Grass
	INFO_BLOCK_CUBE(2, 0), // 3: Dirt
	INFO_BLOCK_CUBE(0, 1), // 4: Cobblestone
	INFO_BLOCK_CUBE(4, 0), // 5: Wood Planks
	INFO_BLOCK_PLANT(15, 0), // 6: Sapling
	INFO_BLOCK_CUBE(1, 1), // 7: Bedrock
	INFO_BLOCK_CUBE(14, 0), // 8: Water (Flowing)
	INFO_BLOCK_CUBE(14, 0), // 9: Water (Still)
	INFO_BLOCK_CUBE(14, 1), // 10: Lava (Flowing)
	INFO_BLOCK_CUBE(14, 1), // 11: Lava (Still)
	INFO_BLOCK_CUBE(2, 1), // 12: Sand
	INFO_BLOCK_CUBE(3, 1), // 13: Gravel
	INFO_BLOCK_CUBE(0, 2), // 14: Gold Ore
	INFO_BLOCK_CUBE(1, 2), // 15: Iron Ore
	INFO_BLOCK_CUBE(2, 2), // 16: Coal Ore
	INFO_BLOCK_CUBE_TB(4, 1, 5, 1, 5, 1), // 17: Wood Log
	INFO_BLOCK_CUBE(6, 1), // 18: Leaves
	INFO_BLOCK_CUBE(0, 3), // 19: Sponge
	INFO_BLOCK_CUBE(1, 3), // 20: Glass

	INFO_BLOCK_WOOL(0),
	INFO_BLOCK_WOOL(1),
	INFO_BLOCK_WOOL(2),
	INFO_BLOCK_WOOL(3),
	INFO_BLOCK_WOOL(4),
	INFO_BLOCK_WOOL(5),
	INFO_BLOCK_WOOL(6),
	INFO_BLOCK_WOOL(7),
	INFO_BLOCK_WOOL(8),
	INFO_BLOCK_WOOL(9),
	INFO_BLOCK_WOOL(10),
	INFO_BLOCK_WOOL(11),
	INFO_BLOCK_WOOL(12),
	INFO_BLOCK_WOOL(13),
	INFO_BLOCK_WOOL(14),
	INFO_BLOCK_WOOL(15),

	INFO_BLOCK_PLANT(13, 0), // 37: Dandelion
	INFO_BLOCK_PLANT(12, 0), // 38: Rose
	INFO_BLOCK_PLANT(13, 1), // 39: Brown Mushroom
	INFO_BLOCK_PLANT(12, 1), // 40: Red Mushroom

	INFO_BLOCK_CUBE_TB(8, 2, 8, 1, 8, 3), // 41: Block of Gold
	INFO_BLOCK_CUBE_TB(7, 2, 7, 1, 7, 3), // 42: Block of Iron

	INFO_BLOCK_CUBE_TB(5, 0, 6, 0, 6, 0), // 43: Double Slab
	INFO_BLOCK_CUBE_TB(5, 0, 6, 0, 6, 0), // 44: Slab

	INFO_BLOCK_CUBE(7, 0), // 45: Brick
	INFO_BLOCK_CUBE_TB(8, 0, 9, 0, 10, 0), // 46: TNT
	INFO_BLOCK_CUBE_TB(3, 2, 4, 0, 4, 0), // 47: Bookshelf
	INFO_BLOCK_CUBE(4, 2), // 48: Mossy Cobblestone 
	INFO_BLOCK_CUBE(5, 2), // 49: Obsidian
};
