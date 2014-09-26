#include "BlockData.h"
#include "BlockMaps.h"

BlockData::BlockData()
{
	
}

BlockData::~BlockData()
{
	
}

BlockData *BlockData::Create(uint32_t blkid, uint32_t)
{
	switch(blkid)
	{
		case BLOCK_STONE:
		case BLOCK_GRASS:
		case BLOCK_DIRT:
		case BLOCK_COBBLESTONE:
		case BLOCK_PLANKS:
		case BLOCK_BEDROCK:
		case BLOCK_SAND:
		case BLOCK_GRAVEL:
		case BLOCK_LOG:
		case BLOCK_LEAVES:
		case BLOCK_SPONGE:
		case BLOCK_GLASS:
		case BLOCK_LAPIS_ORE:
		case BLOCK_LAPIS_BLOCK:
		case BLOCK_DISPENSER:
		case BLOCK_SANDSTONE:
		case BLOCK_NOTEBLOCK:
		case BLOCK_STICKY_PISTON:
		case BLOCK_PISTON:
		case BLOCK_WOOL:
		case BLOCK_PISTON_EXTENSION:
		case BLOCK_GOLD_BLOCK:
		case BLOCK_IRON_BLOCK:
		case BLOCK_DOUBLE_STONE_SLAB:
		case BLOCK_BRICK_BLOCK:
		case BLOCK_TNT:
		case BLOCK_BOOKSHELF:
		case BLOCK_MOSSY_COBBLESTONE:
		case BLOCK_OBSIDIAN:
		case BLOCK_MOB_SPAWNER:
		case BLOCK_DIAMOND_ORE:
		case BLOCK_DIAMOND_BLOCK:
		case BLOCK_CRAFTING_TABLE:
		case BLOCK_FARMLAND:
		case BLOCK_FURNACE:
		case BLOCK_LIT_FURNACE:
		case BLOCK_REDSTONE_ORE:
		case BLOCK_LIT_REDSTONE_ORE:
		case BLOCK_ICE:
		case BLOCK_SNOW:
		case BLOCK_CACTUS:
		case BLOCK_CLAY:
		case BLOCK_JUKEBOX:
		case BLOCK_PUMPKIN:
		case BLOCK_NETHERRACK:
		case BLOCK_SOUL_SAND:
		case BLOCK_GLOWSTONE:
		case BLOCK_LIT_PUMPKIN:
		case BLOCK_STAINED_GLASS:
		case BLOCK_STONEBRICK:
		case BLOCK_BROWN_MUSHROOM_BLOCK:
		case BLOCK_RED_MUSHROOM_BLOCK:
		case BLOCK_MELON_BLOCK:
		case BLOCK_MYCELIUM:
		case BLOCK_NETHER_BRICK:
		case BLOCK_END_PORTAL_FRAME:
		case BLOCK_END_STONE:
		case BLOCK_REDSTONE_LAMP:
		case BLOCK_LIT_REDSTONE_LAMP:
		case BLOCK_DOUBLE_WOODEN_SLAB:
		case BLOCK_EMERALD_ORE:
		case BLOCK_EMERALD_BLOCK:
		case BLOCK_COMMAND_BLOCK:
		case BLOCK_BEACON:
		case BLOCK_REDSTONE_BLOCK:
		case BLOCK_QUARTZ_BLOCK:
		case BLOCK_DROPPER:
		case BLOCK_STAINED_HARDENED_CLAY:
		case BLOCK_LEAVES2:
		case BLOCK_LOG2:
		case BLOCK_HAY_BLOCK:
		case BLOCK_HARDENED_CLAY:
		case BLOCK_COAL_BLOCK:
		case BLOCK_PACKED_ICE:
			return new SolidBlockData();
		default:
			return new UnknownBlockData();
	}
	
	// unreachable >:(
	return nullptr;
}

uint32_t UnknownBlockData::toVerticies(double *, double, double, double)
{
	return 0;
}

uint32_t SolidBlockData::toVerticies(double *buff, double xoff, double zoff, double yoff)
{
	double cube[] = {
		-0.5f,-0.5f,-0.5f, // triangle 1 : begin
		-0.5f,-0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f, // triangle 1 : end
		0.5f, 0.5f,-0.5f, // triangle 2 : begin
		-0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f, // triangle 2 : end
		0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,
		0.5f, 0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f,-0.5f,
		0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f,-0.5f, 0.5f,
		0.5f,-0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f,-0.5f,-0.5f,
		0.5f, 0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f,-0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f,-0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f,-0.5f, 0.5f
	};
	
	for(int i = 0; i < sizeof(cube) / sizeof(double); i++)
	{
		int m = i % 3;
		if(m == 0)
			buff[i] = cube[i] + xoff;
		else if(m == 1)
			buff[i] = cube[i] + yoff;
		else if(m == 2)
			buff[i] = cube[i] + zoff;
	}
	
	return sizeof(cube) / sizeof(double);
}

