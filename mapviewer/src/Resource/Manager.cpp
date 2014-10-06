#include "Resource/Manager.h"
#include "Resource/Resource.h"
#include "Resource/Bitmap.h"
#include "Resource/Atlas.h"
#include <Resource/Model.h>
#include <Renderer.h>
#include <MCModel.h>

#include <allegro5/allegro.h>
#include "NBT_Debug.h"

#include <stdio.h>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

ResourceManager::ResourceManager(Renderer *renderer, const std::string &base, const std::string &bmpSubPath, const std::string &modelSubPath) :
	baseResourcePath_(base), bitmapSubPath_(bmpSubPath), modelSubPath_(modelSubPath)
{
	NBT_Debug("begin");
	atlas_ = new Atlas(renderer, 128, 16);
	renderer_ = renderer;
	NBT_Debug("end");
}

ResourceManager::~ResourceManager()
{
	delete atlas_;
	
	for(auto doc: jsonDocCache_)
	{
		delete doc.second;
	}
	
	for(auto res: resToUnload_)
	{
		delete res.second;
	}
	
	for(auto res: resources_)
	{
		delete res.second;
	}
}

std::string ResourceManager::bmpPath(const std::string &name)
{
	return bitmapSubPath_ + "/" + name;
}

std::string ResourceManager::resPath(const std::string &name)
{
	return baseResourcePath_ + "/" + name;
}

std::string ResourceManager::modelPath(const std::string &name)
{
	return modelSubPath_ + "/" + name;
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
		delete res;
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

Resource::ID ResourceManager::getModel(const std::string& name)
{
	std::string path = modelPath(name);
	std::string fpath = resPath(path);
	
	ResourceModel *model = nullptr;
	
	Resource::ID rID = findID(fpath);
	if(rID != Resource::INVALID_ID)
	{
		Resource *res = findResource(rID);
		if(res)
		{
			if(res->type() != Resource::ModelType)
				return Resource::INVALID_ID;
			
			res->ref();
			return res->id();
		}
		
		auto it = resToUnload_.find(fpath);
		if(it != resToUnload_.end() && it->second == nullptr)
			return Resource::INVALID_ID;
		
		model = dynamic_cast<ResourceModel*>(it->second);
		if(!model)
			return Resource::INVALID_ID;
		
		resToUnload_.erase(it);
	}
	else
	{
		MCModel *mcmod = MCModel::Create(name, this);
		if(!mcmod)
		{
			NBT_Debug("failed to load model %s", fpath.c_str());
			return Resource::INVALID_ID;
		}
		
		model = new ResourceModel(fpath, mcmod);
	}
	
	resources_.emplace(model->id(), model);
	nameToIDMap_.emplace(fpath, model->id());
	
	return model->id();
}

bool ResourceManager::putModel(Resource::ID id)
{
	return putResource(id);
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


rapidjson::Document *ResourceManager::getJson(const std::string& name)
{
	// cache ftw
	std::string path = resPath(modelPath(name));
	NBT_Debug("attempting to fetch %s json", name.c_str());
	
	auto it = jsonDocCache_.find(path);
	if(it != jsonDocCache_.end() && it->second)
		return it->second;
	
	path += ".json";
	FILE *fh = fopen(path.c_str(), "r");
	if(!fh)
		return nullptr;
	
	char buffer[1024];
	
	rapidjson::FileReadStream istream{ fh, buffer, sizeof(buffer) };
	
	rapidjson::Document *doc = new rapidjson::Document;
	jsonDocCache_.emplace(path, doc);
	
	doc->ParseStream(istream);
	if(doc->HasParseError())
	{
		const char *errstr = rapidjson::GetParseError_En(doc->GetParseError());
		NBT_Error("error parsing json: %s", errstr);
	}
	
	return jsonDocCache_[path];
}


