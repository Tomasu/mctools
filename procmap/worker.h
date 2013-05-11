#ifndef WORKER_H_GUARD
#define WORKER_H_GUARD

#include <thread>
#include <mutex>
#include <queue>
#include "Map.h"
#include "NBT_Debug.h"

class Region;

struct Worker {
	int id;
	std::thread *thread;
	bool done;
	template<typename _Callable, typename... _Args>
	Worker(_Callable &&fn, _Args&&... __args) : id(next_thread_id++), thread(0), done(false)
	{
		thread = new std::thread(std::forward<_Callable>(fn), this, std::forward<_Args>(__args)...);
	}
	
	private:
		static int next_thread_id;
};

extern std::mutex worker_mutex;
extern std::vector<Worker *> workers;
extern std::queue<Region *> region_queue;

template<typename _Callable, typename... _Args>
bool worker_process_map(Map *map, _Callable &&fn, _Args&&... __args)
{
	NBT_Debug("fill region_queue for workers");
	Region *region = map->firstRegion();
	while(region != 0)
	{
		region_queue.push(region);
		region = map->nextRegion();
	}
	
	int hw_concurrency = std::thread::hardware_concurrency() / 2;
	int concurrent_workers =  hw_concurrency ? hw_concurrency+1 : 2;
	NBT_Debug("start %i workers", concurrent_workers);
	for(int i = 0; i < concurrent_workers; i++)
	{
		workers.push_back( new Worker(fn, std::forward<_Args>(__args)...) );
	}

	NBT_Debug("wait for workers to finish");
	for(int i = 0; i < concurrent_workers; i++)
	{
		workers[i]->thread->join();
		delete workers[i];
	}
	
	workers.clear();
	
	NBT_Debug("workers finished.");
	
	return true;
}

#endif /* WORKER_H_GUARD */
