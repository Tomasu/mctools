#include <vector>
#include <unistd.h>
#include "clean.h"
#include "MCRegion.h"
#include "Map.h"
#include "Chunk.h"
#include "BitMap.h"
#include "worker.h"

uint32_t total_keep_chunks = 0;
uint32_t total_deleted_chunks = 0;
	
bool clean_region(MCRegion *region, BitMap *chunkBitMap)
{
	uint32_t keep_chunks = 0;
	uint32_t deleted_chunks = 0;

	if(region->load())
	{
		bool save_region = false;
		std::vector<Chunk *> chunks = region->chunks();
	
		for(auto &chunk: chunks)
		{
			if(!chunk)
				continue;
			
			if(!chunkBitMap->get(chunk->x(), chunk->z()))
			{
				//NBT_Debug("delete chunk");
				region->deleteChunk(chunk);
				save_region = true;
				deleted_chunks++;
				total_deleted_chunks++;
			}
			else
			{
				keep_chunks++;
				total_keep_chunks++;
			}
		}
		
		if(!region->chunkCount())
		{
			NBT_Debug("delete region %ix%i", region->x(), region->z());
			unlink(region->filePath().c_str());
		}
		else
		{
			if(save_region)
			{
				if(!region->save())
				{
					NBT_Error("failed to save region %ix%i", region->x(), region->z());
				}
			}
		}
		
		NBT_Debug("region %ix%i: kept %u chunks, deleted %u chunks.", region->x(), region->z(), keep_chunks, deleted_chunks);
		region->unload();
	}
	
	return true;
}

void worker_fn_clean(Worker *worker, BitMap *bitMap)
{
	while(region_queue.size() > 0)
	{
		worker_mutex.lock();
		MCRegion *region = region_queue.front();
		region_queue.pop();
		worker_mutex.unlock();
		
		NBT_Debug("worker %i begin: region %ix%i", worker->id, region->x(), region->z());
		if(!clean_region(region, bitMap))
		{
			NBT_Error("failed to load region %ix%i", region->x(), region->z());
		}
		NBT_Debug("worker %i end: region %ix%i", worker->id, region->x(), region->z());
	}
	
	NBT_Debug("worker %i done.", worker->id);
}

bool clean_map(Map *map, BitMap *bitMap)
{
	bool ret = worker_process_map(map, worker_fn_clean, bitMap);
	
	NBT_Debug("kept %u chunks, deleted %u chunks.", total_keep_chunks, total_deleted_chunks);

	return ret;
}
