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

ChunkData *ChunkData::Create(Chunk *c, ResourceManager *resourceManager)
{
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
	assert(num_sections <= MAX_SLICES);

	ChunkData *cdata = new ChunkData(c->x(), c->z());
	
	CUSTOM_VERTEX *dptr = data; // reset dptr, reuse data memory.
	
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
					
					if(blkid != BLOCK_AIR)
						NBT_Debug("blk: %i", blkid);
					
					if(!BlockData::isSolid(blkid))
						continue;
					
					bool up_solid = false, down_solid = false, north_solid = false, west_solid = false, south_solid = false, east_solid = false;
					
					//if(!BlockData::isTranslucent(block_data[idx])
					if(idx_down  < 4096 && idx_down  >= 0)
						down_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_down));
					
					if(idx_up < 4096 && idx_up >= 0)
						up_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_up));
					
					if(idx_east  < 4096 && idx_east  >= 0)
						east_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_east));
					
					if(idx_west  < 4096 && idx_west  >= 0)
						west_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_west));
					
					if(idx_north < 4096 && idx_north >= 0)
						north_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_north));
					
					if(idx_south < 4096 && idx_south >= 0)
						south_solid = BlockData::isSolidForCull(BlockData::ID(block_data, add_data, idx_south));
					
					//NBT_Debug("up:%i down:%i north:%i east:%i south:%i west:%i", up_solid, down_solid, north_solid, east_solid, south_solid, west_solid);
					
					if(up_solid && down_solid && north_solid && east_solid && south_solid && west_solid)
					{
						NBT_Debug(
							"up[%i]: %s, down[%i]: %s, north[%i]: %s, east[%i]: %s, south[%i]: %s, west[%i]: %s",
							idx_up, BlockName(block_data[idx_up]), idx_down, BlockName(block_data[idx_down]), idx_north, BlockName(block_data[idx_north]), 
							idx_east, BlockName(block_data[idx_east]), idx_south, BlockName(block_data[idx_south]), idx_west, BlockName(block_data[idx_west])
						);
						//NBT_Debug("block is surrounded, skip.");
						continue;
					}
					// FIXME: create a cache of these things.
					
					BlockData *block = BlockData::Create(block_data[idx], 0);
					if(!block)
						continue;
					
					std::string resName = "blocks/";
					std::string texName = BlockTexName(block_data[idx], 0);
					
					resName += texName;
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
					
					uint32_t num_idx = block->toVerticies(dptr, xPos + dx, zPos + dz, y + dy, tx_xfact, tx_yfact, tx_x, tx_y, tx_page);
					//NBT_Debug("%s nidx: %i", BlockName(block_data[idx], 0), num_idx);
					dptr += num_idx;
					total_size += num_idx;
					
					delete block;
					
				}
			}
		}
		
#ifdef VIEWER_USE_MORE_VBOS
		if(!cdata->fillSlice(section_y, data, total_size))
			NBT_Warn("failed to fill slice %i???", y);
#endif
		
	}
	
#ifndef VIEWER_USE_MORE_VBOS
	if(!cdata->fillSlice(0, data, total_size))
		NBT_Warn("failed to fill chunk data");
#endif
	
	delete[] data;
	
	return cdata;
}
