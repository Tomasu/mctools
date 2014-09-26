#include "ChunkData.h"
#include "BlockData.h"
#include "Chunk.h"
#include <NBT_Tag_Compound.h>
#include <NBT_Tag_List.h>
#include <NBT_Tag_Byte_Array.h>
#include <NBT_Tag_Int.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
   float x, y, z;
} CUSTOM_VERTEX;

ChunkData::ChunkData(float *data, uint32_t size) : vbo_(0), size_(size)
{
	ALLEGRO_VERTEX_ELEMENT elements[] = {
		{ ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_3, offsetof(CUSTOM_VERTEX, x) },
		{ 0, 0, 0 }
	};
	
	vtxdecl_ = al_create_vertex_decl(elements, sizeof(CUSTOM_VERTEX));
	
	vbo_ = al_create_vertex_buffer(vtxdecl_, data, size_/3, 0);
}

ChunkData::~ChunkData()
{
	al_destroy_vertex_buffer(vbo_);
	al_destroy_vertex_decl(vtxdecl_);
}

void ChunkData::draw()
{
	al_draw_vertex_buffer(vbo_, NULL, 0, size_/3, ALLEGRO_PRIM_TRIANGLE_LIST);
}

ChunkData *ChunkData::Create(Chunk *c)
{
	float *data = new float[MAX_DATA_SIZE];
	if(!data)
		return nullptr;
	
	float *dptr = data;
	
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
	
	for(auto &section_tag: sections->items())
	{
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
					
					// FIXME: create a cache of these things.
					BlockData *block = BlockData::Create(block_data[idx], 0);
					
					uint32_t num_idx = block->toVerticies(dptr, xPos + dx, zPos + dz, y + dy);
					//NBT_Debug("nidx: %i", num_idx);
					dptr += num_idx;
					total_size += num_idx;
					
					delete block;
					
				}
			}
		}
	}
	
	ChunkData *cdata = new ChunkData(data, total_size);
	delete data;
	
	return cdata;
}
