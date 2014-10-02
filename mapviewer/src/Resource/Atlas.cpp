#include "Resource/Atlas.h"
#include "Resource/AtlasSheet.h"
#include "Renderer.h"

#include "NBT_Debug.h"

Atlas::Atlas(Renderer *renderer, uint32_t maxSz, uint32_t gridSz) : sheetSize_(maxSz), gridSize_(gridSz), renderer_(renderer)
{
	NBT_Debug("begin");
	curSheet_ = new AtlasSheet(sheets_.size(), maxSz, gridSz);
	sheets_.push_back(curSheet_);
	//renderer->setShaderSampler(curSheet_);
	NBT_Debug("end");
}

Atlas::~Atlas()
{
	for(auto &it: sheets_)
	{
		delete it;
	}
}

ResourceBitmap *Atlas::load(const std::string &path)
{
	std::string fpath = path + ".png";
	//NBT_Debug("attempting to load %s", fpath.c_str());
	
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_BITMAP);
	
	//al_set_new_bitmap_flags(ALLEGRO_NO_PREMULTIPLIED_ALPHA);
	ALLEGRO_BITMAP *bmp = al_load_bitmap(fpath.c_str());
	if(!bmp)
	{
		//NBT_Debug("failed to load %s", fpath.c_str());
		return nullptr;
	}
	
	AtlasSheet *sheet = getSheet();
	Item sheetItem;
	if(!sheet->allocItem(bmp, &sheetItem))
		return nullptr;

	al_destroy_bitmap(bmp);
	
	ResourceBitmap *res = new ResourceBitmap(path);
	sheetItem.id = res->id();
	
	idItemMap_.emplace(res->id(), sheetItem);
	
	al_restore_state(&state);
	
	return res;
}

void Atlas::remove(Resource::ID id)
{
	auto it = idItemMap_.find(id);
	if(it != idItemMap_.end() || it->second.id == Resource::INVALID_ID)
		return;
	
	Item item = it->second;
	if(item.sheet >= sheets_.size())
		return;
	
	AtlasSheet *sheet = sheets_[item.sheet];
	sheet->removeItem(&item);
	
	idItemMap_.erase(id);
	
	//idSheetMap_.emplace(res->id(), )
}

AtlasSheet *Atlas::getSheet()
{
	if(!curSheet_->isFull())
		return curSheet_;
	
	AtlasSheet *newSheet = new AtlasSheet(sheets_.size(), sheetSize_, gridSize_);
	sheets_.push_back(newSheet);
	curSheet_ = newSheet;
	renderer_->setShaderSampler(curSheet_);
	
	return curSheet_;
}

bool Atlas::getItem(Resource::ID id, Item *item)
{
	auto it = idItemMap_.find(id);
	if(it == idItemMap_.end() || it->second.id == Resource::INVALID_ID)
	{
		NBT_Debug("id %i not found", id);
		return false;
	}
	
	if(it->second.sheet >= sheets_.size())
	{
		NBT_Debug("item sheet invalid: %li > %i", it->second.sheet, sheets_.size()-1);
		return false;
	}
	
	*item = it->second;
	
	return true;
}