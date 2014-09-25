#include "worker.h"
#include <mutex>
#include <queue>
#include <pthread.h>
#include "main.h"


int Worker::next_thread_id = 1;
std::mutex worker_mutex;
std::vector<Worker *> workers;
std::queue<MCRegion *> region_queue;

