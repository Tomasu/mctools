#ifndef CLEAN_H_GUARD
#define CLEAN_H_GUARD

#include <cstdint>

class Map;
class BitMap;

extern uint32_t total_keep_chunks;
extern uint32_t total_deleted_chunks;

bool clean_map(Map *map, BitMap *bitMap);

#endif /* CLEAN_H_GUARD */
