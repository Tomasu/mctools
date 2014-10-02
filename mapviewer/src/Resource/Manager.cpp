#include "Resource/Manager.h"
#include "Resource/Resource.h"
#include "Resource/Bitmap.h"
#include "Resource/Atlas.h"
#include <Renderer.h>

#include <allegro5/allegro.h>
#include "NBT_Debug.h"

ResourceManager::ResourceManager(Renderer *renderer, const std::string &base, const std::string &bmpSubPath) :
	baseResourcePath_(base), bitmapSubPath_(bmpSubPath)
{
	NBT_Debug("begin");
	atlas_ = new Atlas(renderer, 64, 16);
	renderer_ = renderer;
	NBT_Debug("end");
}

ResourceManager::~ResourceManager()
{
	delete atlas_;
}

std::string ResourceManager::bmpPath(const std::string &name)
{
	return bitmapSubPath_ + "/" + name;
}

std::string ResourceManager::resPath(const std::string &name)
{
	return baseResourcePath_ + "/" + name;
}

Resource *ResourceManager::findResource(Resource::ID id)
{
	auto it = resources_.find(id);
	if(it == resources_.end() || it->second == nullptr)
		return nullptr;
	
	return it->second;
}

Resource::ID ResourceManager::findID(const std::string &path)
{
	auto it = nameToIDMap_.find(path);
	if(it == nameToIDMap_.end() || it->second == Resource::INVALID_ID)
		return Resource::INVALID_ID;
	
	return it->second;
}

Resource::ID ResourceManager::getResource(Resource::Type type, const std::string &str)
{
	auto it = nameToIDMap_.find(str);
	if(it == nameToIDMap_.end() || it->second == 0)
		return Resource::INVALID_ID;
	
	switch(type)
	{
		case Resource::BitmapType:
			return getBitmap(bmpPath(str));
			
		default:
			return Resource::INVALID_ID;
	}
	
	// shouldn't reach here
	return Resource::INVALID_ID;
}

Resource::ID ResourceManager::refResource(Resource::Type type, const std::string &name)
{
	std::string path = baseResourcePath_ + "/";
	
	switch(type)
	{
		case Resource::BitmapType:
			path += bmpPath(name);
			break;
			
		default:
			path += name;
	}
	
	auto it = nameToIDMap_.find(path);
	if(it == nameToIDMap_.end() || it->second == Resource::INVALID_ID)
		return false;
	
	if(!refResource(it->second))
		return Resource::INVALID_ID;
	
	return it->second;
}

bool ResourceManager::refResource(Resource::ID id)
{
	Resource *res = findResource(id);
	if(res == nullptr)
		return false;
	
	res->ref();
	
	return true;
}

bool ResourceManager::putResource(Resource::ID id)
{
	Resource *res = findResource(id);
	if(res == nullptr)
		return false;
	
	// if we're pinned, gtfo
	if(!res->inUse() && res->isPinned())
		return false;
	
	res->deref();
		
	// if we're not in use, and not pinned, remove the resource.
	if(!res->inUse() && !res->isPinned())
	{
		resToUnload_.emplace(res->path(), res);
		resources_.erase(id);
		return true;
	}
	
	return false;
}

bool ResourceManager::resourceExists(Resource::ID id)
{
	auto it = resources_.find(id);
	if(it == resources_.end() || it->second == nullptr)
		return false;
	
	return true;
}

bool ResourceManager::resourceExists(Resource::Type type, const std::string &name)
{
	std::string path = baseResourcePath_ + "/";

	switch(type)
	{
		case Resource::BitmapType:
			path += bmpPath(name);
			break;
			
		default:
			break;
	}
	
	if(findID(path) == Resource::INVALID_ID)
		return false;
	
	return true;
}

bool ResourceManager::getAtlasItem(Resource::ID id, Atlas::Item *item)
{
	return atlas_->getItem(id, item);
}

Resource::ID ResourceManager::getBitmap(const std::string &name)
{
	std::string path = bmpPath(name);
	std::string fpath = resPath(path);
	ResourceBitmap *bmp = nullptr;
	
	Resource::ID rID = findID(fpath);
	if(rID != Resource::INVALID_ID)
	{
	//	NBT_Debug("found resource by path");
		Resource *res = findResource(rID);
		if(res)
		{
			if(res->type() != Resource::BitmapType)
			{
		//		NBT_Debug("found resource is not a bitmap?");
				return Resource::INVALID_ID;
			}
			
			res->ref();
			return res->id();
		}
		
		// if we still have an id, even though we didn't find the resource in the table,
		//  the resource is still in the unload table, get it from there.
		
		auto it = resToUnload_.find(fpath);
		if(it != resToUnload_.end() && it->second == nullptr)
		{
			// somehow it wasn't actually found...
			return Resource::INVALID_ID;
		}
		
		bmp = dynamic_cast<ResourceBitmap*>(it->second);
		if(!bmp)
			return Resource::INVALID_ID;
		
		resToUnload_.erase(it);
	}
	else
	{
		bmp = atlas_->load(fpath);
		if(!bmp)
			return Resource::INVALID_ID;
	}
	
	resources_.emplace(bmp->id(), bmp);
	nameToIDMap_.emplace(fpath, bmp->id());
	
	return bmp->id();
}

bool ResourceManager::putBitmap(Resource::ID id)
{
	if(!putResource(id))
		return false;
	
	atlas_->remove(id);
	return true;
}

bool ResourceManager::resourceIsPinned(Resource::ID id)
{
	Resource *res = findResource(id);
	if(!res)
		return false;
	
	return res->isPinned();
}

void ResourceManager::pinResource(Resource::ID id)
{
	Resource *res = findResource(id);
	if(!res)
		return;
	
	res->pin();
}

void ResourceManager::unpinResource(Resource::ID id)
{
	Resource *res = findResource(id);
	if(!res)
		return;
	
	res->unpin();
}

void ResourceManager::unsetAtlasUniforms()
{
	for(uint32_t i = 0; i < atlas_->numSheets(); i++)
	{
		AtlasSheet *sheet = atlas_->getSheet(i);
		renderer_->unsetShaderSampler(sheet);
	}
}

bool ResourceManager::setAtlasUniforms()
{
	//NBT_Debug("begin");
	
	for(uint32_t i = 0; i < atlas_->numSheets(); i++)
	{
		AtlasSheet *sheet = atlas_->getSheet(i);
		if(!renderer_->setShaderSampler(sheet))
		{
	//		NBT_Debug("end");
			return false;
		}
	}
	
	//NBT_Debug("end");
	return true;
}

