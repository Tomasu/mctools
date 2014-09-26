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

ChunkData::ChunkData(double *data, uint32_t size) : vbo_(0), size_(size), data_(data)
{
	glGenBuffers(1, &vbo_);
	
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
 
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, size_, data_, GL_STATIC_DRAW);
}

ChunkData::~ChunkData()
{
	glDeleteBuffers(1, &vbo_);
}

void ChunkData::draw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		size_ / 3,          // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
}

ChunkData *ChunkData::Create(Chunk *c)
{
	double *data = new double[MAX_DATA_SIZE];
	if(!data)
		return nullptr;
	
	double *dptr = data;
	
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
					
					uint32_t num_idx = block->toVerticies(dptr, xPos + dx, y + dy, zPos + dz);
					dptr += num_idx;
					total_size += num_idx;
					
					delete block;
					
				}
			}
		}
	}
	
	// make a copy that isnt 6MB in size. loooool.
	double *comp_data = new double[total_size];
	memcpy(comp_data, data, total_size*sizeof(double));
	delete data;
	
	ChunkData *cdata = new ChunkData(comp_data, total_size);
	return cdata;
}
