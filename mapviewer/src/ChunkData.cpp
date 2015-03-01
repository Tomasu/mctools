#include <stddef.h>

#include "ChunkData.h"
#include "BlockData.h"

#include "Resource/Atlas.h"
#include "Resource/Manager.h"
#include "Resource/Model.h"
#include "Resource/ModelVariant.h"

#include "Model/Model.h"
#include "Model/Variant.h"
#include "Model/Element.h"

#include "Chunk.h"
#include "ChunkSection.h"
#include "Block.h"
#include "BlockMaps.h"
#include "BlockInfo.h"

#include "CustomVertex.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <cstdio>

ChunkData::ChunkData(int32_t x, int32_t z) : x_(x), z_(z)
{
	ALLEGRO_VERTEX_ELEMENT elements[] = {
		{ ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_3, offsetof(CustomVertex, pos) },
		{ ALLEGRO_PRIM_TEX_COORD, ALLEGRO_PRIM_FLOAT_2, offsetof(CustomVertex, txcoord) },
		{ ALLEGRO_PRIM_USER_ATTR, ALLEGRO_PRIM_FLOAT_1, offsetof(CustomVertex, tx_page) },
		{ ALLEGRO_PRIM_COLOR_ATTR, ALLEGRO_PRIM_FLOAT_4, offsetof(CustomVertex, color) },
		{ 0, 0, 0 }
	};
	
	vtxdecl_ = al_create_vertex_decl(elements, sizeof(CustomVertex));
	if(!vtxdecl_)
		NBT_Debug("failed to create vertex decl");
	
	memset(slice_, 0, sizeof(slice_));
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

bool ChunkData::fillSlice(int slice_idx, CustomVertex* data, uint32_t vtx_count)
{
	//NBT_Debug("slice_idx:%i", slice_idx);
	assert(slice_idx >= 0 && slice_idx < MAX_SLICES);
	
	auto &slice = slice_[slice_idx];
	assert(slice.vbo == nullptr); // if true, we have a duplicate section :o
	
	slice.vtx_count = vtx_count;
	
	slice.vbo = al_create_vertex_buffer(vtxdecl_, data, slice.vtx_count, 0);
	if(!slice.vbo)
		NBT_Debug("failed to create vertex buffer :(");
	
	//NBT_Debug("new chunk slice[%i]: size:%.02fMB", slice_idx, ((double)slice.vtx_count*sizeof(CustomVertex))/1024.0/1024.0);
	
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
		
		//al_draw_vertex_buffer(slice.vbo, 0, 0, slice.vtx_count-1, ALLEGRO_PRIM_TRIANGLE_LIST);
		al_draw_vertex_buffer(slice.vbo, 0, 0, slice.vtx_count, ALLEGRO_PRIM_TRIANGLE_LIST);
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

ChunkData *ChunkData::Create(Chunk *chunk, ResourceManager *resourceManager)
{
	const uint32_t DATA_VTX_COUNT = MAX_VERTS / MAX_SLICES;
	CustomVertex *data = (CustomVertex*)malloc(sizeof(CustomVertex) * DATA_VTX_COUNT);
	if(!data)
		return nullptr;
		
	memset(data, 0, DATA_VTX_COUNT * sizeof(CustomVertex));
	
	uint32_t total_size = 0;
	
	// TODO: maybe allow putting more than one section per slice if we end up with more than 16 sections.
	//  currently minecraft only uses 16 sections per chunk.
	//assert(num_sections <= MAX_SLICES);

	int32_t x_off = chunk->x() * 16;
	int32_t z_off = chunk->z() * 16;
	
	ChunkData *cdata = new ChunkData(chunk->x(), chunk->z());
	
	CustomVertex *dptr = data; // reset dptr, reuse data memory.
	
	for(uint32_t i = 0; i < chunk->sectionCount(); i++)
	{
		ChunkSection *section = chunk->getSection(i);
		if(!section)
			continue;
		
		//NBT_Debug("new section[%i]: %p", i, section);
		
		int32_t y = section->y() * 16;
		//int32_t y = section_y * 16;
		
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
					BlockAddress baddr;
					
					if(!chunk->getBlockAddress(x_off + dx, y + dy, z_off + dz, &baddr))
					{
						NBT_Debug("failed to find block %i, %i, %i", x_off + dx, y + dy, z_off + dz);
						assert(nullptr);
						continue;
					}
					
					//NBT_Debug("block %i,%i,%i: %s", x_off+dx, y+dy, z_off+dz, baddr.toString().c_str());
					
					BlockInfo bi;
					chunk->getBlockInfo(baddr, &bi);
					
					//NBT_Debug("blockInfo: %i:%i:%s", bi.id, bi.data, bi.state_name);
					
					Resource::ID rid = resourceManager->getModelVariant(bi);
					if(rid == Resource::INVALID_ID)
					{
						//NBT_Debug("failed to get model %i:%i:%s", bi.id, bi.data, bi.state_name);
						continue;
					}
					
					//if(bi.id != BLOCK_AIR && bi.id != BLOCK_BARRIER)
					//	NBT_Debug("block:%i:%i %s", bi.id, bi.data, bi.state_name);
					
					ResourceModelVariant *var = resourceManager->getModelVariantResource(rid);
					uint32_t vertex_count = var->getVertexCount();
					CustomVertex *verticies = var->getVertexData();
					
					for(uint32_t i = 0; i < vertex_count; i++)
					{
						CustomVertex &v = verticies[i], &cv = dptr[i];
						float xoff = cdata->x() + dx, yoff = y + dy, zoff = cdata->z() + dz;
						
						cv.pos = { v.pos.f1 + xoff, v.pos.f2 + yoff, v.pos.f3 + zoff };
						//cv.txcoord = { (v.txcoord.f1 * tx_xfact + tx_x), 1-(v.txcoord.f2 * tx_yfact + tx_y) };
						cv.txcoord = { v.txcoord.f1 , 1-v.txcoord.f2 };
						// { 0.25 + 0.25 * v.txcoord.f1,  0.25 + 0.25 * v.txcoord.f2 }; 
						//NBT_Debug("tex: %f, %f", cv.txcoord.f1, cv.txcoord.f2);
						cv.tx_page = v.tx_page;
						cv.color = v.color;
					}
					
					dptr += vertex_count;
					total_size += vertex_count;
				}
			}
		}
		
#ifdef VIEWER_USE_MORE_VBOS
		if(total_size > 0)
			if(!cdata->fillSlice(section->y(), data, total_size))
				NBT_Warn("failed to fill slice %i???", y);
#endif
		
	}
	
#ifndef VIEWER_USE_MORE_VBOS
	if(total_size)
		if(!cdata->fillSlice(0, data, total_size))
			NBT_Warn("failed to fill chunk data");
#endif
	
	free(data);
	
	return cdata;
}
