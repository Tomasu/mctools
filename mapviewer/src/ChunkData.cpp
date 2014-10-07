#include "ChunkData.h"
#include "BlockData.h"
#include <Resource/Atlas.h>
#include <Resource/Manager.h>
#include "Chunk.h"
#include "Block.h"
#include "BlockMaps.h"
#include "CustomVertex.h"
#include <NBT_Tag_Compound.h>
#include <NBT_Tag_List.h>
#include <NBT_Tag_Byte_Array.h>
#include <NBT_Tag_Int.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

ChunkData::ChunkData(int32_t x, int32_t z) : x_(x), z_(z)
{
	ALLEGRO_VERTEX_ELEMENT elements[] = {
		{ ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_3, offsetof(CUSTOM_VERTEX, pos) },
		{ ALLEGRO_PRIM_TEX_COORD, ALLEGRO_PRIM_FLOAT_2, offsetof(CUSTOM_VERTEX, txcoord) },
		{ ALLEGRO_PRIM_USER_ATTR, ALLEGRO_PRIM_FLOAT_1, offsetof(CUSTOM_VERTEX, tx_page) },
		{ ALLEGRO_PRIM_COLOR_ATTR, ALLEGRO_PRIM_FLOAT_4, offsetof(CUSTOM_VERTEX, color) },
		{ 0, 0, 0 }
	};
	
	vtxdecl_ = al_create_vertex_decl(elements, sizeof(CUSTOM_VERTEX));
	if(!vtxdecl_)
		NBT_Debug("failed to create vertex decl");
	
	memset(slice_, 0, sizeof(slice_));
	
	//for(int i = 0; i < size; i++)
	//{
	//	NBT_Debug("uv: %f,%f", data[i].txcoord.f1, data[i].txcoord.f2);
	//}
	
}

ChunkData::~ChunkData()
{
	for(int i = 0; i < MAX_SLICES; i++)
	{
		auto &slice = slice_[i];
		al_destroy_vertex_buffer(slice.vbo);
	}
	
	al_destroy_vertex_decl(vtxdecl_);
}

bool ChunkData::fillSlice(int slice_idx, CUSTOM_VERTEX* data, uint32_t vtx_count)
{
	//NBT_Debug("slice_idx:%i", slice_idx);
	assert(slice_idx >= 0 && slice_idx < MAX_SLICES);
	
	auto &slice = slice_[slice_idx];
	assert(slice.vbo == nullptr); // if true, we have a duplicate section :o
	
	slice.vtx_count = vtx_count;
	
	slice.vbo = al_create_vertex_buffer(vtxdecl_, data, slice.vtx_count, 0);
	if(!slice.vbo)
		NBT_Debug("failed to create vertex buffer :(");
	
	NBT_Debug("new chunk slice[%i]: size:%.02fMB", slice_idx, ((double)slice.vtx_count*sizeof(CUSTOM_VERTEX))/1024.0/1024.0);
	
	return true;
}


void ChunkData::draw(ALLEGRO_TRANSFORM *trans)
{
	for(int32_t i = 0; i < MAX_SLICES; i++)
	{
		auto &slice = slice_[i];
		if(!slice.vbo)
			break;
		
		// do translation here >:(
		
		ALLEGRO_TRANSFORM local_transform;
		al_copy_transform(&local_transform, trans);
		al_translate_transform_3d(&local_transform, 0.0, -slice.y, 0.0);
		al_use_transform(&local_transform);
		
		al_draw_vertex_buffer(slice.vbo, 0, 0, slice.vtx_count-1, ALLEGRO_PRIM_TRIANGLE_LIST);
		//al_draw_vertex_buffer(vbo_, tex, 0, size_, ALLEGRO_PRIM_TRIANGLE_LIST);
	}
}

struct BLOCK_SIDES {
	uint8_t top : 1;
	uint8_t bottom : 1;
	uint8_t north : 1;
	uint8_t east : 1;
	uint8_t south : 1;
	uint8_t west : 1;
	uint8_t dummy : 2;
};

ChunkData *ChunkData::Create(Chunk *c, ResourceManager *resourceManager)
{
	NBT_Debug("Entering ChunkData::Create");
	const uint32_t DATA_VTX_COUNT = MAX_VERTS / MAX_SLICES;
	CUSTOM_VERTEX *data = new CUSTOM_VERTEX[DATA_VTX_COUNT];
	if(!data)
		return nullptr;
		
	memset(data, 0, DATA_VTX_COUNT * sizeof(CUSTOM_VERTEX));
	
	NBT_Tag_Compound *nbt = c->nbt()->getCompound("Level");
	//NBT_Debug("%ix%i level name: %s children:%i", c->x(), c->z(), nbt->name().c_str(), nbt->count());
	NBT_Tag_List *sections_tag = (NBT_Tag_List*)nbt->get("Sections");
	int32_t xPos = nbt->getInt("xPos");
	int32_t zPos = nbt->getInt("zPos");
	
	uint32_t total_size = 0;
	
	if(!sections_tag)
	{
		NBT_Debug("no sections tag?");
		return nullptr;
	}

	auto &sections = sections_tag->items();
	uint32_t num_sections = sections.size();
	
	// TODO: maybe allow putting more than one section per slice if we end up with more than 16 sections.
	//  currently minecraft only uses 16 sections per chunk.
	//assert(num_sections <= MAX_SLICES);

	ChunkData *cdata = new ChunkData(c->x(), c->z());
	
	CUSTOM_VERTEX *dptr = data; // reset dptr, reuse data memory.
	
	//BLOCK_SIDES cull_sides[16*16][16][16];
	//memset(cull_sides, 0x00, sizeof(cull_sides));
	
	uint8_t cull_mask[16*16][16][16];
	memset(cull_mask, 0x00, sizeof(cull_mask));
	
	for(uint32_t i = 0; i < num_sections; i++)
	{
		NBT_Tag_Compound *section = (NBT_Tag_Compound *)sections[i];
		
		NBT_Tag_Byte_Array *blocks = section->getByteArray("Blocks");
		NBT_Tag_Byte_Array *add = section->getByteArray("Add");
		
		int32_t section_y = section->getByte("Y");
		int32_t y = section_y * 16;
		
		uint8_t *block_data = blocks->data();
		uint8_t *add_data = add ? add->data() : nullptr;
		
		for(int dy = 0; dy < 16; dy++)
		{
			for(int dz = 0; dz < 16; dz++)
			{
				for(int dx = 0; dx < 16; dx++)
				{
					int idx = dy*16*16 + dz*16 + dx;
					
					int idx_down = (dy-1)*16*16 + dz*16 + dx;
					int idx_up = (dy+1)*16*16 + dz*16 + dx;
					int idx_north = dy*16*16 + (dz-1)*16 + dx;
					int idx_south = dy*16*16 + (dz+1)*16 + dx;
					int idx_west = dy*16*16 + dz*16 + (dx-1);
					int idx_east = dy*16*16 + dz*16 + (dx+1);
					
					
					uint32_t blkid = BlockData::ID(block_data, add_data, idx);
					
					
					if(!BlockData::isSolid(blkid))
					{
						//bs.all = 0;
						/*
						cull_sides[y+dy][dz][dx].top = 1;
						cull_sides[y+dy][dz][dx].bottom = 1;
						cull_sides[y+dy][dz][dx].north = 1;
						cull_sides[y+dy][dz][dx].east = 1;
						cull_sides[y+dy][dz][dx].south = 1;
						cull_sides[y+dy][dz][dx].west = 1;
						*/
						continue;
					}
					
					if(idx_up >= 0 && idx_up < 4096)
					{
						cull_mask[y+dy+1][dz][dx] |= BlockData::FACE_DOWN;
					}
					
					if(idx_down >= 0 && idx_down < 4096)
					{
						cull_mask[y+dy-1][dz][dx] |= BlockData::FACE_UP;
					}
					
					if(idx_north >= 0 && idx_north < 4096)
					{
						cull_mask[y+dy][dz-1][dx] |= BlockData::FACE_SOUTH;
					}
					
					if(idx_east >= 0 && idx_east < 4096)
					{
						cull_mask[y+dy][dz][dx+1] |= BlockData::FACE_WEST;
					}
					
					if(idx_south >= 0 && idx_south < 4096)
					{
						cull_mask[y+dy][dz+1][dx] |= BlockData::FACE_NORTH;
					}
					
					if(idx_west >= 0 && idx_west < 4096)
					{
						cull_mask[y+dy][dz][dx-1] |= BlockData::FACE_WEST;
					}
				}
			}
		}
	}
	
	for(uint32_t i = 0; i < num_sections; i++)
	{
		NBT_Tag_Compound *section = (NBT_Tag_Compound *)sections[i];
		
		NBT_Tag_Byte_Array *blocks = section->getByteArray("Blocks");
		NBT_Tag_Byte_Array *add = section->getByteArray("Add");
		
		int32_t section_y = section->getByte("Y");
		int32_t y = section_y * 16;
		
		uint8_t *block_data = blocks->data();
		uint8_t *add_data = add ? add->data() : nullptr;
		
#ifdef VIEWER_USE_MORE_VBOS
		dptr = data;
		total_size = 0;
#endif
		
		for(int dy = 0; dy < 16; dy++)
		{
			for(int dz = 0; dz < 16; dz++)
			{
				for(int dx = 0; dx < 16; dx++)
				{
					int idx = dy*16*16 + dz*16 + dx;
					
					int idx_down = (dy-1)*16*16 + dz*16 + dx;
					int idx_up = (dy+1)*16*16 + dz*16 + dx;
					int idx_north = dy*16*16 + (dz-1)*16 + dx;
					int idx_south = dy*16*16 + (dz+1)*16 + dx;
					int idx_west = dy*16*16 + dz*16 + (dx-1);
					int idx_east = dy*16*16 + dz*16 + (dx+1);
					
					float tx_xfact = 0.0, tx_yfact = 0.0, tx_page = 0.0, tx_x = 0.0, tx_y = 0.0;
					
					uint32_t blkid = BlockData::ID(block_data, add_data, idx);
					
					//if(!BlockData::isSolidForCull(blkid))
					//	continue;
					
				//	NBT_Debug("sides: %x", block_sides[dy][dz][dx].all);
					
					/*
					if(cull_sides[y+dy][dz][dx].top == 1 && cull_sides[y+dy][dz][dx].bottom == 1
						&& cull_sides[y+dy][dz][dx].north == 1 && cull_sides[y+dy][dz][dx].east == 1 
						&& cull_sides[y+dy][dz][dx].south == 1 && cull_sides[y+dy][dz][dx].west == 1 
					)
						continue;
					*/
					
					// FIXME: create a cache of these things.

					std::string resName = "blocks/";
					std::string texName = BlockTexName(block_data[idx], 0);
					
					resName += texName;
					
					std::string modName = "block/";
					modName += BlockTexName(block_data[idx], 0);
					
					Resource::ID rid = resourceManager->getModel(modName);
						
					BlockData *block = BlockData::Create(block_data[idx], 0);
					if(!block)
						continue;
					

					if(texName.length())
					{
						//NBT_Debug("blockName: %s", resName.c_str());
						Resource::ID res_id = resourceManager->getBitmap(resName);
						if(res_id != Resource::INVALID_ID)
						{
							//NBT_Debug("got a resource: %i", res_id);
							
							resourceManager->pinResource(res_id); // keep it around, we're gonna need it.
		
							Atlas::Item item;
							if(resourceManager->getAtlasItem(res_id, &item))
							{
								Atlas *atlas = resourceManager->getAtlas();
								
								tx_page = item.sheet + 1;
								//NBT_Debug("tx_page: %f", tx_page);
								tx_xfact = (float)atlas->gridSize() / (float)atlas->sheetSize();
								tx_yfact = (float)atlas->gridSize() / (float)atlas->sheetSize();
								tx_x = item.x * tx_xfact;
								tx_y = item.y * tx_yfact;
							//	NBT_Debug("%i ix:%f, iy:%f, xf:%f, yf:%f x:%f y:%f", block_data[idx], item.x, item.y, tx_xfact, tx_yfact, tx_x, tx_y);
							}
						}
						else
						{
							//NBT_Debug("failed to load resource :( %s", resName.c_str());
						}
					}
					
					//NBT_Debug("Entering toVerticies");
					uint32_t num_idx = block->toVerticies(dptr, xPos + dx, zPos + dz, y + dy, tx_xfact, tx_yfact, tx_x, tx_y, tx_page, cull_mask[y+dy][dz][dx]);
					//NBT_Debug("Exiting toVerticies");
					//NBT_Debug("%s nidx: %i", BlockName(block_data[idx], 0), num_idx);
					dptr += num_idx;
					total_size += num_idx;
					
					delete block;
					
				}
			}
		}
		
#ifdef VIEWER_USE_MORE_VBOS
		if(total_size > 0)
			if(!cdata->fillSlice(section_y, data, total_size))
				NBT_Warn("failed to fill slice %i???", y);
#endif
		
	}
	
#ifndef VIEWER_USE_MORE_VBOS
	if(total_size)
		if(!cdata->fillSlice(0, data, total_size))
			NBT_Warn("failed to fill chunk data");
#endif
	
	delete[] data;
	
	return cdata;
}
