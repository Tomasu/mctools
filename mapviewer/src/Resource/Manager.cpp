#include "Resource/Manager.h"
#include "Resource/Resource.h"
#include "Resource/Bitmap.h"
#include "Resource/Atlas.h"
#include "Resource/Model.h"
#include "Renderer.h"
#include "MCModel/Model.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <physfs.h>

#include "NBT_Debug.h"

#include <cstdio>
#include <ctime>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

ResourceManager::ResourceManager(Renderer *renderer, const std::string &base, const std::string &bmpSubPath, const std::string &modelSubPath, const std::string &blockstateSubPath) :
	baseResourcePath_(base), bitmapSubPath_(bmpSubPath), modelSubPath_(modelSubPath), blockstateSubPath_(blockstateSubPath)
{
	renderer_ = renderer;
	
	NBT_Debug("!");
}

bool ResourceManager::init(const char *argv0)
{
	NBT_Debug("begin");
	
	ALLEGRO_PATH *data_path = nullptr;
	const char *data_path_cstr = nullptr;
	
	ALLEGRO_PATH *mc_path = nullptr, *jar_path = nullptr;
	const char *jar_path_cstr = nullptr;
	
	if (!PHYSFS_init(argv0))
	{
		NBT_Debug("failed to initialize Physfs");
		return false;
	}
	
	const PHYSFS_ArchiveInfo **i;

	for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
	{
		NBT_Debug("Supported archive: [%s], which is [%s].\n",
					(*i)->extension, (*i)->description);
	}
	
	data_path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);
	if(!data_path)
	{
		NBT_Debug("failed to get user data path");
		goto init_err;
	}
	
	data_path_cstr = al_path_cstr(data_path, ALLEGRO_NATIVE_PATH_SEP);
	if(!al_filename_exists(data_path_cstr))
	{
		if(!al_make_directory(data_path_cstr))
		{
			NBT_Debug("failed to create data path (%s): %s", data_path_cstr, strerror(al_get_errno()));
			goto init_err;
		}
	}
	
	NBT_Debug("write path: %s", data_path_cstr);
	
	if(PHYSFS_setWriteDir(data_path_cstr) == 0)
	{
		NBT_Debug("failed to set physfs write dir (%s): %s", data_path_cstr, PHYSFS_getLastError());
		goto init_err;
	}
	
	if(PHYSFS_mount(data_path_cstr, nullptr, 0) == 0)
	{
		NBT_Debug("failed to mount write dir (%s): %s", data_path_cstr, PHYSFS_getLastError());
		goto init_err;
	}
	
	mc_path = locateMinecraftData();
	if(!mc_path)
		goto init_err;
	
	NBT_Debug("mc path: %s", al_path_cstr(mc_path, ALLEGRO_NATIVE_PATH_SEP));
	
	jar_path = locateMinecraftJar(mc_path);
	if(!jar_path)
	{
		NBT_Debug("failed to locate minecraft jar");
		goto init_err;
	}
	
	jar_path_cstr = al_path_cstr(jar_path, ALLEGRO_NATIVE_PATH_SEP);
	NBT_Debug("jar path: %s", jar_path_cstr);
	
	if(PHYSFS_mount(jar_path_cstr, nullptr, 1) == 0)
	{
		NBT_Debug("failed to mount jar (%s): %s", jar_path_cstr, PHYSFS_getLastError());
		goto init_err;
	}
	
	al_set_physfs_file_interface();
	
	atlas_ = new Atlas(renderer_, 512, 16);
	
	NBT_Debug("end");
	return true;
	
init_err:
	if(PHYSFS_isInit())
		PHYSFS_deinit();
	
	if(data_path)
		al_destroy_path(data_path);
	
	return false;
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

std::string ResourceManager::blockstatePath(const std::string &name)
{
	return blockstateSubPath_ + "/" + name;
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
		//delete res;
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

ResourceModel *ResourceManager::getModelResource(Resource::ID rID)
{
	ResourceModel *model = nullptr;
	
	Resource *res = findResource(rID);
	if(!res)
	{
		auto it = resToUnload_.find(res->path());
		if(it != resToUnload_.end() && it->second == nullptr)
			return nullptr;
		
		res = it->second;
		if(!res || res->type() != Resource::ModelType)
			return nullptr;
		
		resToUnload_.erase(it);
		resources_.emplace(res->id(), res);
	}
	else if(res->type() != Resource::ModelType)
		return nullptr;
	
	res->ref();
	
	return (ResourceModel*)res;
}

Resource::ID ResourceManager::getModel(const std::string& name)
{
	std::string path = blockstatePath(name);
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
		MCModel::Model *mcmod = MCModel::Model::Create(name, this);
		if(!mcmod)
		{
			NBT_Debug("failed to load model %s %s", name.c_str(), fpath.c_str());
			//return Resource::INVALID_ID;
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

rapidjson::Document *ResourceManager::getBlockstateJson(const std::string &name)
{
	return getJson(resPath(blockstatePath(name)));
}

rapidjson::Document *ResourceManager::getModelJson(const std::string& name)
{
	// cache ftw
	std::string path = resPath(modelPath(name));
	
	return getJson(resPath(modelPath(name)));
}

rapidjson::Document *ResourceManager::getJson(const std::string& p)
{
	std::string path = p;
	rapidjson::Document *doc = nullptr;
	char *buffer = nullptr;
	
	//NBT_Debug("attempting to fetch %s.json", path.c_str());
	
	// cache ftw
	auto it = jsonDocCache_.find(path);
	if(it != jsonDocCache_.end() && it->second)
		return it->second;
	
	path += ".json";
	
	ALLEGRO_FILE *fh = al_fopen(path.c_str(), "r");
	if(!fh)
	{
		NBT_Debug("failed to open json (%s): %s", path.c_str(), strerror(al_get_errno()));
		return nullptr;
	}
	
	int64_t fsize = al_fsize(fh);
	if(fsize < 1)
	{
		//NBT_Debug("json file is empty?");
		goto getJson_err;
	}
	
	buffer = (char *)al_malloc(fsize + 1);
	if(!buffer)
		goto getJson_err;
	
	memset(buffer, 0, fsize+1);
	
	if(al_fread(fh, buffer, fsize) != fsize)
	{
		//NBT_Error("failed to read %i bytes from file", fsize);
		goto getJson_err;
	}
	
	doc = new rapidjson::Document;
	
	doc->Parse(buffer);
	
	if(doc->HasParseError())
	{
		const char *errstr = rapidjson::GetParseError_En(doc->GetParseError());
		NBT_Error("error parsing json: %s", errstr);
		
		size_t err_off = doc->GetErrorOffset();
		char err_buff[1024];
		memset(err_buff, 0, sizeof(err_buff));
		
		const char *eoline = strchr(buffer+err_off, '\n');
		if(!eoline)
			eoline = buffer+fsize;
		
		size_t err_minsz = std::min((long)sizeof(err_buff)-1, eoline-buffer-1);
		memcpy(err_buff, buffer+err_off, err_minsz);
		
		NBT_Error("at[%i]: %s", err_off, err_buff);
		
		goto getJson_err;
	}
	
	jsonDocCache_.emplace(path, doc);
	
	//NBT_Debug("got json!");
	return jsonDocCache_[path];
	
getJson_err:
	if(doc)
		delete doc;
	
	if(buffer)
		al_free(buffer);

	if(fh)
		al_fclose(fh);
	
	return nullptr;
}

ALLEGRO_PATH *ResourceManager::locateMinecraftData()
{
	NBT_Debug("begin");
	
	ALLEGRO_PATH *mc_path = nullptr;
	
#if ALLEGRO_WINDOWS
	const char *appdata = getenv("APPDATA");
	mc_path = al_create_path_for_directory(appdata);
	
	al_append_path_component(mc_path, ".minecraft");
#elif ALLEGRO_MACOSX
	mc_path = al_get_standard_path(ALLEGRO_USER_HOME_PATH);
	if(!mc_path)
		goto err;
	
	al_append_path_component(mc_path, "Library");
	al_append_path_component(mc_path, "Application Support");
	al_append_path_component(mc_path, "minecraft");
#else
	// assume unix
	mc_path = al_get_standard_path(ALLEGRO_USER_HOME_PATH);
	if(!mc_path)
		goto err;
	
	al_append_path_component(mc_path, ".minecraft");
#endif
	
	NBT_Debug("end");
	
	return mc_path;
	
err:
	if(mc_path)
		al_destroy_path(mc_path);
	
	return nullptr;
}

ALLEGRO_PATH *ResourceManager::locateMinecraftJar(ALLEGRO_PATH *base_path)
{
	typedef std::vector< MCVersion > VersionVector;
	VersionVector version_vec;
	VersionVector::reverse_iterator it;
	MCVersion selected_version;
	
	ALLEGRO_FS_ENTRY *versions_dir = nullptr, *entry = nullptr;
	const char *versions_path = nullptr;
	std::string str;
	
	ALLEGRO_PATH *jar_path = al_clone_path(base_path);
	if(!jar_path)
		goto err;
	
	al_append_path_component(jar_path, "versions");
	versions_path = al_path_cstr(jar_path, ALLEGRO_NATIVE_PATH_SEP);
	
	if(!al_filename_exists(versions_path))
	{
		// don't really need this, we really only support minecrafts
		// whith the versions folder... but what the heck.
		
		al_set_path_filename(jar_path, "minecraft.jar");
		if(al_filename_exists(al_path_cstr(jar_path, ALLEGRO_NATIVE_PATH_SEP)))
			return jar_path;
		else
			return nullptr;
	}
	
	versions_dir = al_create_fs_entry(versions_path);
	if(!versions_dir)
		goto err;
	
	if(!al_open_directory(versions_dir))
		goto err;
	
	
	while((entry = al_read_directory(versions_dir)) != nullptr)
	{
		time_t releaseTime = 0;
		struct tm tm;
		rapidjson::Value releaseDateValue;
		rapidjson::Value releaseTypeValue;
		rapidjson::Value releaseIDValue;
		
		const char *entry_name = al_get_fs_entry_name(entry);
		ALLEGRO_PATH *entry_path = nullptr;
		const char *entry_fname = nullptr;
		std::string entry_fname_str;
		
		rapidjson::Document *version_json = nullptr;
		
		uint32_t entry_mode = al_get_fs_entry_mode(entry);
		
		if(entry_mode & ALLEGRO_FILEMODE_HIDDEN && !(entry_mode & ALLEGRO_FILEMODE_ISDIR)) // ignore hidden and non directory entries
			continue;
		
		ALLEGRO_PATH *json_path = al_clone_path(jar_path);
		if(!json_path)
			continue;
		
		entry_path = al_create_path(entry_name);
		if(!entry_path)
			goto free_loop_data;
		
		entry_fname = al_get_path_filename(entry_path);
		
		al_append_path_component(json_path, entry_fname);
		
		al_set_path_filename(json_path, entry_fname);
		
		version_json = getJson(al_path_cstr(json_path, ALLEGRO_NATIVE_PATH_SEP));
		if(!version_json)
			goto free_loop_data;
		
		releaseDateValue = (*version_json)["releaseTime"];
		releaseTypeValue = (*version_json)["type"];
		releaseIDValue = (*version_json)["id"];

		if(!strptime(releaseDateValue.GetString(), "%Y-%m-%dT%T%z", &tm))
		{
			NBT_Debug("failed to parse releaseTime: %s", releaseDateValue.GetString());
			goto free_loop_data;
		}
		
		releaseTime = mktime(&tm);
		
		{
		MCVersion ver = MCVersion(releaseIDValue.GetString(), releaseTime, strcmp(releaseTypeValue.GetString(), "snapshot")==0);
		NBT_Debug("found ver: %s", ver.str().c_str());
		version_vec.push_back(ver);
		}
		
	free_loop_data:
		
		if(version_json)
			delete version_json;
		
		if(json_path)
			al_destroy_path(json_path);
		
		if(entry_path)
			al_destroy_path(entry_path);
	}
	
	al_close_directory(versions_dir);
	
	if(version_vec.size() < 1)
	{
		NBT_Debug("failed to find any mc versions?");
		goto err;
	}
	
	std::sort(version_vec.begin(), version_vec.end());
	
	it = version_vec.rbegin();
	selected_version = *it;
	
	while(it != version_vec.rend())
	{
		selected_version = *it;
		NBT_Debug("test version %s: %s", selected_version.str().c_str(), al_path_cstr(jar_path, ALLEGRO_NATIVE_PATH_SEP));
		
		if(it->isSnapshot())
		{
			it++;
			continue;
		}
		
		it++;
	
		if(!selected_version.isValid())
		{
			NBT_Debug("selected_version is invalid");
			break;
		}
		
		ALLEGRO_PATH *ver_path = al_clone_path(jar_path);
		al_append_path_component(ver_path, selected_version.str().c_str());
		
		str = selected_version.str() + ".jar";
		al_set_path_filename(ver_path, str.c_str());
		
		if(!al_filename_exists(al_path_cstr(ver_path, ALLEGRO_NATIVE_PATH_SEP)))
		{
			NBT_Debug("%s does not exist??", al_path_cstr(ver_path, ALLEGRO_NATIVE_PATH_SEP));
			al_destroy_path(ver_path);
			continue;
		}
		
		NBT_Debug("selected version %s: %s", selected_version.str().c_str(), al_path_cstr(jar_path, ALLEGRO_NATIVE_PATH_SEP));
		
		al_destroy_path(jar_path);
		return ver_path;
	}
	
err:
	if(jar_path)
		al_destroy_path(jar_path);
	
	return nullptr;
}
