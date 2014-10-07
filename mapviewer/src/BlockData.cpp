#include "BlockData.h"
#include <Resource/Manager.h>
#include "BlockMaps.h"
#include "Block.h"
#include "CustomVertex.h"
#include <algorithm>

namespace
{
	#define color al_map_rgb(0,0,0)

	CUSTOM_VERTEX face_vtx[36] = {
		//front face
		//upper left triangle (viewed face on)
		{ { -0.5, 0.5, 0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ { -0.5,-0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5, 0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ {  0.5, 0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ { -0.5,-0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5,-0.5, 0.5 }, { 1.0, 0.0 }, 0.0, color },	//lower right
		//left face (viewers left when looking at front face
		//upper left triangle (viewed face on)
		{ { -0.5, 0.5,-0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ { -0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ { -0.5, 0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ { -0.5, 0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ { -0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ { -0.5,-0.5, 0.5 }, { 1.0, 0.0 }, 0.0, color },	//lower right
		//right face (viewers right when looking at front face
		//upper left triangle (viewed face on)
		{ {  0.5, 0.5, 0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ {  0.5,-0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ {  0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ {  0.5,-0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5,-0.5,-0.5 }, { 1.0, 0.0 }, 0.0, color },	//lower right
		//back face
		//upper left triangle (viewed face on)
		{ {  0.5, 0.5,-0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ {  0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ { -0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ { -0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ {  0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ { -0.5,-0.5,-0.5 }, { 1.0, 0.0 }, 0.0, color },	//lower right
		//top face
		//upper left triangle (viewed face on)
		{ { -0.5, 0.5,-0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ { -0.5, 0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ {  0.5, 0.5,-0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ { -0.5, 0.5, 0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5, 0.5, 0.5 }, { 1.0, 0.0 }, 0.0, color },	//lower right
		//bottom face
		//upper left triangle (viewed face on)
		{ { -0.5,-0.5, 0.5 }, { 0.0, 1.0 }, 0.0, color },	//upper left
		{ { -0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5,-0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right
		//lower right triangle (viewed face on)
		{ {  0.5,-0.5, 0.5 }, { 1.0, 1.0 }, 0.0, color },	//upper right again
		{ { -0.5,-0.5,-0.5 }, { 0.0, 0.0 }, 0.0, color },	//lower left
		{ {  0.5,-0.5,-0.5 }, { 1.0, 0.0 }, 0.0, color }	//lower right
	};
	#undef color
}

BlockData::BlockData()
{
	
}

BlockData::~BlockData()
{
	
}

bool BlockData::isSolidForCull(uint32_t blockid)
{
	return isSolid(blockid) && !isTranslucent(blockid);
}

bool BlockData::isTranslucent(uint32_t blockid)
{
	switch(blockid)
	{
		case BLOCK_AIR:
		case BLOCK_WATER:
		case BLOCK_FLOWING_WATER:
		case BLOCK_LAVA:
		case BLOCK_FLOWING_LAVA:
			return true;
			
		default:
			return false;
	}
	
	return false;
}

bool BlockData::isSolid(uint32_t blockid)
{
	if(blockid == BLOCK_AIR)
		return false;
	
	switch(blockid)
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
		case BLOCK_COAL_ORE:
		case BLOCK_IRON_ORE:
		case BLOCK_GOLD_ORE:
		case BLOCK_PACKED_ICE:
		case BLOCK_FLOWING_LAVA:
		case BLOCK_LAVA:
		case BLOCK_FLOWING_WATER:
		case BLOCK_WATER:
			return true;
		default:
			//NBT_Debug("!solid: %s", BlockName(blockid));
			return false;
	}
	
	return false;
}

BlockData *BlockData::Create(uint32_t blkid, uint32_t)
{
	if(isSolid(blkid))
		return new SolidBlockData();
	else
		return new UnknownBlockData();
	
	// unreachable >:(
	return nullptr;
}

uint32_t UnknownBlockData::toVerticies(CUSTOM_VERTEX*,  float, float, float, float, float, float, float, float, uint8_t)
{
	return 0;
}

uint32_t SolidBlockData::toVerticies(CUSTOM_VERTEX* buff, float xoff, float zoff, float yoff, float tx_xfact, float tx_yfact, float tx_x, float tx_y, float tx_page, uint8_t side_mask)
{
	uint8_t r = 0, g = 0, b = 0;
	uint8_t side_count = 0;
	

//	ALLEGRO_COLOR color = al_map_rgb(0,0,0);
	
	for (int32_t i = 0; i < 6; ++i)
		if (side_mask & (1 << i))
			++side_count;
	
	uint8_t vtx_count = side_count * 6;
		
	CUSTOM_VERTEX vtx[vtx_count];
	
	for (int32_t i = 0; i < 6; ++i)
	{
		if (side_mask & (1 << i))
			std::copy(face_vtx + (6 * i), face_vtx + (6 * i) + 6, vtx + (6 * i));
	}
	
#undef color
	
	CUSTOM_VERTEX *ptr = buff;
	
	for(int i = 0; i < vtx_count; i++)
	{
		CUSTOM_VERTEX &v = vtx[i], &cv = *ptr;
		ptr++;
		
		cv.pos = { v.pos.f1 + xoff, v.pos.f2 + yoff, v.pos.f3 + zoff };
		cv.txcoord = { (v.txcoord.f1 * tx_xfact + tx_x), 1-(v.txcoord.f2 * tx_yfact + tx_y) };
		// { 0.25 + 0.25 * v.txcoord.f1,  0.25 + 0.25 * v.txcoord.f2 }; 
		//NBT_Debug("tex: %f, %f", cv.txcoord.f1, cv.txcoord.f2);
		cv.tx_page = tx_page;
		cv.color = v.color;
	}
	
	return vtx_count;
}

