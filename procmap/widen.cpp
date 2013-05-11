#include "widen.h"
#include "BitMap.h"
#include "NBT_Debug.h"

BitMap *widen_bitmap(BitMap *bitMap)
{
	int64_t mapWidth = bitMap->width();
	int64_t mapHeight = bitMap->height();
	
	NBT_Debug("widen_bitmap: map w:%lu h:%lu bm size: %lu", mapWidth, mapHeight, bitMap->getSize());
	BitMap *widened = new BitMap(bitMap->top(), bitMap->left(), bitMap->bottom(), bitMap->right());
	for(int64_t x = bitMap->left(); x <= bitMap->right(); x++)
	{
		for(int64_t z = bitMap->top(); z < bitMap->bottom(); z++)
		{
			bool val = bitMap->get(x, z);
			if(val)
			{
				widened->set(x, z);
				
				circleFill(widened, x, z, 10);
			}
		}
	}
	
	return widened;
}