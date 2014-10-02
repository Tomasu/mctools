#ifndef ATLASSHEET_H_GUARD
#define ATLASSHEET_H_GUARD
#include <unordered_map>
#include "Resource.h"
#include "Atlas.h"

class ALLEGRO_BITMAP;
class AtlasBitmap;
class AtlasSheet 
{
	public:
		AtlasSheet(uint32_t id, uint32_t maxSz, uint32_t gridSz);
		~AtlasSheet();
		
		uint32_t id() { return id_; }
		bool allocItem(ALLEGRO_BITMAP *bmp, Atlas::Item *item);
		bool removeItem(Atlas::Item *item);
		
		bool isFull() { return full_; }
		
		ALLEGRO_BITMAP *alBitmap() { return sheet_; }
		
	private:
		uint32_t id_;
		uint32_t size_;
		uint32_t gridSize_;
		bool full_;
		AtlasBitmap *bitmap_;
		ALLEGRO_BITMAP *sheet_;

		bool findFree(Atlas::Item *item);
};

#endif /* ATLASSHEET_H_GUARD */
