#ifndef MAIN_H_GUARD
#define MAIN_H_GUARD

#include "BitMap.h"
#include "Block.h"

extern BitMap *chunkBitMap;
extern const uint32_t keep_block_ids[];
extern uint64_t block_counts[BLOCK_COUNT];

bool keep_block(uint32_t block_id);

#endif /* MAIN_H_GUARD */
