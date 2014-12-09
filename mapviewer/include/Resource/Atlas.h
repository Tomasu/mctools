#ifndef ATLAS_H_GUARD
#define ATLAS_H_GUARD

#include "Resource/Resource.h"
#include "Resource/Bitmap.h"
#include <vector>
#include <unordered_map>

class Renderer;
// TODO: use a rect allocation scheme, and get rid of fixed grid size.
class ALLEGRO_BITMAP;
class AtlasSheet;


class Atlas
{
	public:
		struct Item { uint32_t sheet; Resource::ID id; float x, y, left, top, width, height; };
		
		Atlas(Renderer *renderer, uint32_t maxSz = 1024, uint32_t gridSz = 16);
		~Atlas();
		
		ResourceBitmap *load(const std::string &path);
		void remove(Resource::ID id);
		
		bool getItem(Resource::ID id, Item *item);
		
		// TODO: eventually we may want to support non square textures...
		uint32_t sheetSize() { return sheetSize_; }
		uint32_t gridSize() { return gridSize_; }
		
		float xfact() const { return (float)gridSize_ / (float)sheetSize_; }
		float yfact() const { return (float)gridSize_ / (float)sheetSize_; }
		
		uint32_t numSheets() { return sheets_.size(); }
		AtlasSheet *getSheet(uint32_t idx) { return sheets_[idx]; }
	private:
		uint32_t sheetSize_;
		uint32_t gridSize_;
		
		AtlasSheet *curSheet_;
		std::vector<AtlasSheet *> sheets_;
		
		std::unordered_map<Resource::ID, Item> idItemMap_;
		
		Renderer *renderer_;
		
		AtlasSheet *getSheet();
		
};

#endif /* ATLAS_H_GUARD */
