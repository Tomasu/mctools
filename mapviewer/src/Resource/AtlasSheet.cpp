#include "Resource/AtlasSheet.h"
#include "Resource/AtlasBitmap.h"
#include <allegro5/allegro.h>
#include "NBT_Debug.h"

AtlasSheet::AtlasSheet(uint32_t id, uint32_t maxSz, uint32_t gridSz) :
	id_(id), size_(maxSz), gridSize_(gridSz), full_(false)
{
	NBT_Debug("begin");
	NBT_Debug("size: %i, gridSize_: %i", size_, gridSize_);
	bitmap_ = new AtlasBitmap(size_ / gridSize_);
	sheet_ = al_create_bitmap(size_, size_);
	
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(sheet_);
	al_clear_to_color(al_map_rgba(1.0,1.0,1.0,0));
	al_restore_state(&state);
	NBT_Debug("end");
}

AtlasSheet::~AtlasSheet()
{
	delete bitmap_;
	al_destroy_bitmap(sheet_);
}

bool AtlasSheet::allocItem(ALLEGRO_BITMAP *bmp, Atlas::Item *item)
{
	Atlas::Item newItem;
	
	if(!findFree(&newItem))
		return false;
	
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP);
	
	al_set_target_bitmap(sheet_);
	
	uint32_t sw = al_get_bitmap_width(bmp), sh = al_get_bitmap_height(bmp);
	NBT_Debug("got bitmap of size %ix%i to %f,%f, x:%f, y:%f", sw, sh, newItem.left, newItem.top, newItem.x, newItem.y);
	
	al_draw_bitmap_region(bmp, 0, 0, gridSize_, gridSize_, newItem.left, newItem.top, 0);
	
	al_restore_state(&state);

	// assign individually so we don't overwrite the resource id value which may or may not be set...
	item->sheet = id_;
	item->id = newItem.id;
	item->x = newItem.x;
	item->y = newItem.y;
	item->left = newItem.left;
	item->top = newItem.top;
	item->width = newItem.width;
	item->height = newItem.height;
	
	return true;
}

bool AtlasSheet::removeItem(Atlas::Item *item)
{
	uint32_t x = item->x / gridSize_, y = item->y / gridSize_;
	if(!bitmap_->get(x, y))
		return false;
	
	bitmap_->unset(x, y);
	full_ = false;
	
	return true;
}

bool AtlasSheet::findFree(Atlas::Item *item)
{
	for(uint32_t i = 0; i < size_/gridSize_; i++)
	{
		for(uint32_t j = 0; j < size_/gridSize_; j++)
		{
			if(bitmap_->get(j, i))
				continue;
			
			float left = j * gridSize_, top = i * gridSize_;
			*item = { id_, 0, (float)j, (float)i, left, top, (float)gridSize_, (float)gridSize_ };
			bitmap_->set(j, i);
			NBT_Debug("alloc %ix%i", j, i);
			return true;
		}
	}
	
	return false;
}
