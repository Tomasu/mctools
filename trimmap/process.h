#ifndef PROCESS_H_GUARD
#define PROCESS_H_GUARD

#include <vector>
#include <stdint.h>
class Map;
class BitMap;
bool process_map(Map *map, BitMap *bitMap, const std::vector<uint32_t> &blocks, bool keep = true);

#endif /* PROCESS_H_GUARD */
