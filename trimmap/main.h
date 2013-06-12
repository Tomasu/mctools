#ifndef MAIN_H_GUARD
#define MAIN_H_GUARD

#include "BitMap.h"
#include "Block.h"

extern BitMap *chunkBitMap;
extern uint64_t block_counts[BLOCK_COUNT];

bool has_block(const std::vector<uint32_t> &blocks, uint32_t block_id);

#endif /* MAIN_H_GUARD */
