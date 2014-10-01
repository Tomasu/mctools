#include "BlockData.h"
#include <Resource/Manager.h>
#include "BlockMaps.h"
#include "CustomVertex.h"

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

uint32_t UnknownBlockData::toVerticies(CUSTOM_VERTEX*,  float, float, float, float, float, float, float, float)
{
	return 0;
}

uint32_t SolidBlockData::toVerticies(CUSTOM_VERTEX* buff, float xoff, float zoff, float yoff, float tx_xfact, float tx_yfact, float tx_x, float tx_y, float tx_page)
{
	const int NUM_VERTS_SHARED = 8;
	VF3 verts[NUM_VERTS_SHARED] = {
		{ 0.500000, -0.500000, -0.500000 },
		{ 0.500000, -0.500000, 0.500000 },
		{ -0.500000, -0.500000, 0.500000 },
		{ -0.500000, -0.500000, -0.500000 },
		{ 0.500000, 0.500000, -0.500000 },
		{ 0.500000, 0.500000, 0.500000 },
		{ -0.500000, 0.500000, 0.500000 },
		{ -0.500000, 0.500000, -0.500000 }
	};
	
	VF2 txcs[NUM_VERTS_SHARED] = {
		{ 0.500000, 0.500000 },
		{ 0.500000, 0.000000 },
		{ 0.000000, 0.000000 },
		{ 0.000000, 0.500000 },
		{ 0.500000, 1.000000 },
		{ 0.000000, 1.000000 },
		{ 1.000000, 1.000000 },
		{ 1.000000, 0.500000 }
	};
	
	const int NUM_FACES = 12;
	VI3 vtxFaces[NUM_FACES] = {
		{ 2, 3, 4 },
		{ 8, 7, 6 },
		{ 1, 5, 6 },
		{ 2, 6, 7 },
		{ 7, 8, 4 },
		{ 1, 4, 8 },
		{ 1, 2, 4 },
		{ 5, 8, 6 },
		{ 2, 1, 6 },
		{ 3, 2, 7 },
		{ 3, 7, 4 },
		{ 5, 1, 8 }
	};
	
	
	VI3 txFaces[NUM_FACES] = {
		{ 1, 2, 3 },
		{ 4, 3, 5 },
		{ 5, 6, 4 },
		{ 5, 6, 4 },
		{ 6, 7, 8 },
		{ 1, 2, 3 },
		{ 5, 1, 3 },
		{ 6, 4, 5 },
		{ 3, 5, 4 },
		{ 3, 5, 4 },
		{ 5, 6, 8 },
		{ 5, 1, 3 }
	};
	
	CUSTOM_VERTEX *ptr = buff;
	
	for(int i = 0; i < NUM_FACES; i++)
	{
		VI3 &vtxface = vtxFaces[i];
		VI3 &txface = txFaces[i];
		
		for(int j = 0; j < 3; j++)
		{
			VF3 &vert = verts[vtxface.i[j]-1];
			VF2 &txc = txcs[txface.i[j]-1];
			
			CUSTOM_VERTEX &cv = *ptr;
			ptr++;
			
			cv.pos = { vert.f1 + xoff, vert.f2 + yoff, vert.f3 + zoff };
			cv.txcoord = { ( txc.f1 * tx_xfact + tx_x ), ( txc.f2 * tx_yfact + tx_y ) };
			//cv.txcoord = txc;
			//NBT_Debug("txc: %f,%f", cv.txcoord.f1, cv.txcoord.f2);
			//NBT_Debug("xy: %f, %f, %f", cv.pos.f1, cv.pos.f2, cv.pos.f3);
			cv.tx_page = tx_page;
		}
	}
	
	return NUM_VERTS;
}

