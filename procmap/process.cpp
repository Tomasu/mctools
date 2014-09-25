#include <cstring>
#include "process.h"
#include "NBT_Debug.h"
#include "NBT_Tag_Compound.h"
#include "NBT_Tag_List.h"
#include "NBT_Tag_Byte_Array.h"
#include "MCRegion.h"
#include "Chunk.h"
#include "BitMap.h"
#include "worker.h"
#include "main.h"

bool process_chunk(Chunk *chunk, const std::vector<uint32_t> &blocks_vector, bool keep)
{
	NBT_Tag_Compound *nbt = chunk->nbt()->getCompound("Level");
	//NBT_Debug("%ix%i level name: %s children:%i", chunk->x(), chunk->z(), nbt->name().c_str(), nbt->count());
	NBT_Tag_List *sections = (NBT_Tag_List*)nbt->get("Sections");
	
	//for(auto &key: nbt->keys())
	//{
	//	NBT_Debug("key: %s", key.c_str());
	//}
	
	if(!sections)
	{
		NBT_Debug("no sections tag?");
		return false;
	}
	
	bool keep_chunk = false;
	
	//NBT_Debug("processing %i sections", sections->items().size());
	for(auto &section_tag: sections->items())
	{
		//printf(".");
		NBT_Tag_Compound *section = (NBT_Tag_Compound *)section_tag;
		NBT_Tag_Byte_Array *blocks = section->getByteArray("Blocks");
		NBT_Tag_Byte_Array *add  = section->getByteArray("Add");
		uint8_t *block_data = blocks->data();
		uint8_t *add_data = add ? add->data() : 0;
		
		for(int i = 0; i < blocks->getSize(); i++)
		{
			uint32_t block_id = block_data[i];
			uint32_t add_id = 0;
			if(add_data)
			{
				add_id = add_data[i / 2];
				if(i & 1)
				{
					add_id &= 0xf0;
				}
				else
				{
					add_id <<= 4;
				}
				
				//block_id += add_id;
			}
			
			block_counts[block_id]++;
			
			bool i_can_has_block = has_block(blocks_vector, block_id);
			if(keep)
			{
				if(i_can_has_block)
				{
					keep_chunk = true;
				// NO BREAK! We want to count blocks
				//break;
					NBT_Debug("keep chunk %ix%i because of: %s", chunk->x(), chunk->z(), BlockName(block_id));
					return true;
				}
			}
			else
			{
				if(!i_can_has_block)
				{
					keep_chunk = true;
					NBT_Debug("keep chunk %ix%i because of: %s", chunk->x(), chunk->z(), BlockName(block_id));
					return true;
				}
			}
			//printf("block: %i [%i:%i]\n", block_id + add_id, block_id, add_id);
		}
	}
	//printf("\n");
	
	return keep_chunk;
}

bool process_region(MCRegion *region, BitMap *bitMap, const std::vector<uint32_t> &blocks, bool keep)
{
	if(!region->load())
	{
		printf("failed to load region\n");
		return false;
	}
	
	int keep_chunks = 0;
	int delete_chunks = 0;
	NBT_Debug("processing region %ix%i with %i chunks", region->x(), region->z(), region->chunkCount());
	for(auto &chunk: region->chunks())
	{
		if(!chunk)
			continue;
		
		bool keep_block = process_chunk(chunk, blocks, keep);
		if(!keep_block)
		{
			delete_chunks++;
			//region->deleteChunk(chunk);
			//printf("delete chunk %ix%i\n", chunk->x(), chunk->z());
		}
		else
		{
			keep_chunks++;
			//printf("keep chunk %ix%i\n", chunk->x(), chunk->z());
			
			bitMap->set(chunk->x(), chunk->z());
		}
	}
	
	region->unload();
	
	//printf("keeping %i chunks, deleting %i chunks.\n", keep_chunks, delete_chunks);
	
	return true;
}

void worker_fn_process(Worker *worker, BitMap *bitMap, const std::vector<uint32_t> &blocks, bool keep)
{
	while(region_queue.size() > 0)
	{
		worker_mutex.lock();
		MCRegion *region = region_queue.front();
		region_queue.pop();
		worker_mutex.unlock();
		
		NBT_Debug("worker %i begin: region %ix%i", worker->id, region->x(), region->z());
		if(!process_region(region, bitMap, blocks, keep))
		{
			NBT_Error("failed to load region %ix%i", region->x(), region->z());
		}
		NBT_Debug("worker %i end: region %ix%i", worker->id, region->x(), region->z());
	}
	
	NBT_Debug("worker %i done.", worker->id);
}

bool process_map(Map *map, BitMap *bitMap, const std::vector<uint32_t> &blocks, bool keep)
{
	memset(block_counts, 0, sizeof(block_counts));
	return worker_process_map(map, worker_fn_process, bitMap, blocks, keep);
}
