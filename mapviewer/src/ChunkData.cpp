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

ChunkData::ChunkData(CUSTOM_VERTEX *data, uint32_t size, int32_t x, int32_t z) : vbo_(0), size_(size), x_(x), z_(z)
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
	
	vbo_ = al_create_vertex_buffer(vtxdecl_, data, size_, 0);
	if(!vbo_)
		NBT_Debug("failed to create vertex buffer :(");
	
	NBT_Debug("new chunk: size:%.02fMB", ((double)size_*sizeof(CUSTOM_VERTEX))/1024.0/1024.0);
	
	//for(int i = 0; i < size; i++)
	//{
	//	NBT_Debug("uv: %f,%f", data[i].txcoord.f1, data[i].txcoord.f2);
	//}
	
}

ChunkData::~ChunkData()
{
	al_destroy_vertex_buffer(vbo_);
	al_destroy_vertex_decl(vtxdecl_);
}

void ChunkData::draw()
{
	al_draw_vertex_buffer(vbo_, 0, 0, size_-1, ALLEGRO_PRIM_TRIANGLE_LIST);
	//al_draw_vertex_buffer(vbo_, tex, 0, size_, ALLEGRO_PRIM_TRIANGLE_LIST);
}

ChunkData *ChunkData::Create(Chunk *c, ResourceManager *resourceManager)
{
	CUSTOM_VERTEX *data = new CUSTOM_VERTEX[MAX_VERTS];
	if(!data)
		return nullptr;
	
	CUSTOM_VERTEX *dptr = data;
	
	NBT_Tag_Compound *nbt = c->nbt()->getCompound("Level");
	//NBT_Debug("%ix%i level name: %s children:%i", c->x(), c->z(), nbt->name().c_str(), nbt->count());
	NBT_Tag_List *sections = (NBT_Tag_List*)nbt->get("Sections");
	int32_t xPos = nbt->getInt("xPos");
	int32_t zPos = nbt->getInt("zPos");
	
	uint32_t total_size = 0;
	
	if(!sections)
	{
		NBT_Debug("no sections tag?");
		return nullptr;
	}
	
	int slice_num = 0;
	for(auto &section_tag: sections->items())
	{
		if(slice_num >= 1)
			break;
		
		slice_num++;
		
		NBT_Tag_Compound *section = (NBT_Tag_Compound *)section_tag;
		NBT_Tag_Byte_Array *blocks = section->getByteArray("Blocks");
		int32_t y = section->getByte("Y") * 16;
		
		uint8_t *block_data = blocks->data();
		
		for(int dy = 0; dy < 16; dy++)
		{
			for(int dz = 0; dz < 16; dz++)
			{
				for(int dx = 0; dx < 16; dx++)
				{
					int idx = dy*16*16 + dz*16 + dx;
					float tx_xfact = 0.0, tx_yfact = 0.0, tx_page = 0.0, tx_x = 0.0, tx_y = 0.0;
					
					if(block_data[idx] == BLOCK_AIR)
						continue;
					
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
	}
	
	ChunkData *cdata = new ChunkData(data, total_size, c->x(), c->z());
	delete[] data;
	
	return cdata;
}
