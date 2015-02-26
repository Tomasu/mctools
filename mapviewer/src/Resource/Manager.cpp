#include "Minecraft.h"

#include "Resource/Manager.h"
#include "Resource/Resource.h"
#include "Resource/Bitmap.h"
#include "Resource/Atlas.h"
#include "Resource/Model.h"
#include "Resource/ModelVariant.h"
#include "Renderer.h"

#include "Model/Model.h"
#include "Model/Variant.h"
#include "Model/Element.h"

#include "BlockInfo.h"
#include "CustomVertex.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_primitives.h>
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
	
	NBT_Debug("ctor!");
}

bool ResourceManager::init(Minecraft *mc, const char *argv0)
{
	NBT_Debug("begin");
	
	ALLEGRO_PATH *data_path = nullptr;
	const char *data_path_cstr = nullptr;
	
	std::string mc_path, jar_path;
	
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
	
	mc_path = mc->saves().at(0);
	if(!mc_path.length())
		goto init_err;
	
	NBT_Debug("mc path: %s", mc_path.c_str());
	
	jar_path = mc->selectedJar();
	if(!jar_path.length())
	{
		NBT_Debug("failed to locate minecraft jar");
		goto init_err;
	}
	
	NBT_Debug("jar path: %s", jar_path.c_str());
	
	if(PHYSFS_mount(jar_path.c_str(), nullptr, 1) == 0)
	{
		NBT_Debug("failed to mount jar (%s): %s", jar_path.c_str(), PHYSFS_getLastError());
		goto init_err;
	}
	
	
	
	al_set_physfs_file_interface();
	
	NBT_Debug("create atlas");
	atlas_ = new Atlas(renderer_, 512, 16);
	
	NBT_Debug("create missing block bitmap");
	if(!createMissingBlockBitmap())
	{
		NBT_Debug("failed to create missing block bitmap???");
		goto init_err;
	}
	
	NBT_Debug("end");
	return true;
	
init_err:
	if(atlas_)
		delete atlas_;
	
	if(PHYSFS_isInit())
		PHYSFS_deinit();
	
	if(data_path)
		al_destroy_path(data_path);
	
	return false;
}

ResourceManager::~ResourceManager()
{
	delete atlas_;
	
	//for(auto doc: jsonDocCache_)
	//{
	//	delete doc.second;
	//}
	
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

std::string ResourceManager::modelVariantPath(const std::string& name, uint32_t variant)
{
	std::string path = blockstateSubPath_ + "/";
	path += name + "/";
	path += '0' + variant;
	
	return path;
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

Resource::ID ResourceManager::getLoadedBitmap(const std::string &name, bool fullpath)
{
	std::string fpath = fullpath ? name : resPath(bmpPath(name));
	
	NBT_Debug("begin");
	
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
		
		ResourceBitmap *bmp = dynamic_cast<ResourceBitmap*>(it->second);
		if(!bmp)
			return Resource::INVALID_ID;
		
		resToUnload_.erase(it);
	}
	
	NBT_Debug("end");
	return rID;
}

Resource::ID ResourceManager::getBitmap(const std::string &name)
{
	std::string path = bmpPath(name);
	std::string fpath = resPath(path);
	Resource *bmp = nullptr;
	
	NBT_Debug("begin");
	
	Resource::ID rID = getLoadedBitmap(fpath, true);
	if(rID == Resource::INVALID_ID)
	{
		bmp = atlas_->load(fpath);
		if(!bmp)
			return getLoadedBitmap("missing");
		
		rID = bmp->id();
		
		resources_.emplace(bmp->id(), bmp);
		nameToIDMap_.emplace(fpath, bmp->id());
	}
	
	NBT_Debug("end");
	
	return rID;
}

bool ResourceManager::putBitmap(Resource::ID id)
{
	if(!putResource(id))
		return false;
	
	atlas_->remove(id);
	return true;
}

bool ResourceManager::createMissingBlockBitmap()
{
	NBT_Debug("begin");
	
	ALLEGRO_BITMAP *bmp = al_create_bitmap(atlas_->gridSize(), atlas_->gridSize());
	if(!bmp)
		return false;
	
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_BITMAP);
	
	al_set_target_bitmap(bmp);
	
	for(int i = 0; i < 16; i++)
	{
		ALLEGRO_COLOR color;
		int row = i / 4;
		int col = i % 4;
		
		if((row + col) % 2)
			color = al_map_rgb(0,0,0);
		else
			color = al_map_rgb(255,0,255);
		
		NBT_Debug("draw[%i][%i] at %ix%i to %ix%i", row, col, col * 4, row * 4, col * 4 + 4, row * 4 + 4);
		al_draw_filled_rectangle(col * 4, row * 4, col * 4 + 4, row * 4 + 4, color);
	}
	
	al_restore_state(&state);
	
	std::string path = resPath(bmpPath("missing"));
	
	ResourceBitmap *rb = atlas_->copy(path, bmp);
	if(!rb)
	{
		al_destroy_bitmap(bmp);
		return false;
	}
	
	al_destroy_bitmap(bmp);
	
	resources_.emplace(rb->id(), rb);
	nameToIDMap_.emplace(path, rb->id());
	
	return true;
}

ResourceModel *ResourceManager::getModelResource(Resource::ID rID)
{
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
		Model::Model *mcmod = Model::Model::Create(name);
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

ResourceModelVariant* ResourceManager::getModelVariantResource(Resource::ID id)
{
	Resource *res = findResource(id);
	if(!res)
	{
		// TODO: try and refactor this code into a method that can be shared
		auto it = resToUnload_.find(res->path());
		if(it != resToUnload_.end() && it->second == nullptr)
			return nullptr;
		
		res = it->second;
		if(!res || res->type() != Resource::ModelVariantType)
			return nullptr;
		
		resToUnload_.erase(it);
		resources_.emplace(res->id(), res);
	}
	else if(res->type() != Resource::ModelVariantType)
		return nullptr;
	
	res->ref();
	
	return (ResourceModelVariant*)res;
}

Resource::ID ResourceManager::getModelVariant(const BlockInfo& info)
{
	// FIXME: actually grab correct variant, rather than the first.
	uint32_t variant_id = 0;
	
	std::string path = modelVariantPath(info.state_name, variant_id);
	std::string fpath = resPath(path);
	
	ResourceModelVariant *variant  = nullptr;
	
	Resource::ID rID = findID(fpath);
	if(rID != Resource::INVALID_ID)
	{
		// TODO: refactor this code to pull out as much of the following as is possble.
		Resource *res = findResource(rID);
		if(res)
		{
			if(res->type() != Resource::ModelVariantType)
				return Resource::INVALID_ID;
			
			res->ref();
			return res->id();
		}
		
		auto it = resToUnload_.find(fpath);
		if(it != resToUnload_.end() && it->second == nullptr)
			return Resource::INVALID_ID;
		
		variant = dynamic_cast<ResourceModelVariant*>(it->second);
		if(!variant)
			return Resource::INVALID_ID;
		
		resToUnload_.erase(it);
	}
	else
	{
		Resource::ID mID = getModel(info.state_name);
		if(mID == Resource::INVALID_ID)
			return Resource::INVALID_ID;
	
		ResourceModel *model = getModelResource(mID);
		if(!model || !model->model())
			return Resource::INVALID_ID;

		// FIXME: actually grab correct variant, rather than the first.
		auto model_variant = model->model()->getVariants().at(variant_id);
		
		CustomVertex *vertices = nullptr;
		uint32_t vertex_count = measureVariant(model_variant), vidx = 0;
		
		vertices = new CustomVertex[vertex_count];
		
		for(auto element: model_variant->elements_)
		{
			Model::Element::POINT_MAP pmap_ = element->pmap();
	
			if(element->faces[Model::Face::FACE_UP].direction == Model::Face::FACE_UP)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_UP, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to3(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from4(), uv.p2(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from3(), uv.p1(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to3(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to4(), uv.p4(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from4(), uv.p2(), tx_page);
			}
			
			if(element->faces[Model::Face::FACE_SOUTH].direction == Model::Face::FACE_SOUTH)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_SOUTH, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to2(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to4(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p2(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p2(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to4(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to3(), uv.p4(), tx_page);
			}
			
			if(element->faces[Model::Face::FACE_WEST].direction == Model::Face::FACE_WEST)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_WEST, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to4(), uv.p4(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to2(), uv.p2(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from4(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to4(), uv.p4(), tx_page);
			}
			
			if(element->faces[Model::Face::FACE_NORTH].direction == Model::Face::FACE_NORTH)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_NORTH, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.from1(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from3(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p2(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p2(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from3(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from4(), uv.p4(), tx_page);
			}
			
			if(element->faces[Model::Face::FACE_EAST].direction == Model::Face::FACE_EAST)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_EAST, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.from1(), uv.p2(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from3(), uv.p4(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to3(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from3(), uv.p4(), tx_page);
			}
			
			if(element->faces[Model::Face::FACE_DOWN].direction == Model::Face::FACE_DOWN)
			{
				float tx_page = 0.0;
				Model::Element::UV_MAP uv = mapFaceUV(element, Model::Face::FACE_DOWN, tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from1(), uv.p3(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p4(), tx_page);
				
				vertices[vidx++] = mapCustomVertex(element, pmap_.to1(), uv.p1(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.from2(), uv.p4(), tx_page);
				vertices[vidx++] = mapCustomVertex(element, pmap_.to2(), uv.p2(), tx_page);
			}
		}
		
		variant = new ResourceModelVariant(fpath, vertices, vertex_count);
	}
	
	resources_.emplace(variant->id(), variant);
	nameToIDMap_.emplace(fpath, variant->id());
	
	return variant->id();
}

int32_t ResourceManager::measureVariant(Model::Variant* v)
{
	int32_t vertex_count = 0;
	
	for(auto element: v->elements_)
	{
		if(element->faces[Model::Face::FACE_UP].direction == Model::Face::FACE_UP)
			vertex_count += 6;
		
		if(element->faces[Model::Face::FACE_SOUTH].direction == Model::Face::FACE_SOUTH)
			vertex_count += 6;
		
		if(element->faces[Model::Face::FACE_WEST].direction == Model::Face::FACE_WEST)
			vertex_count += 6;
		
		if(element->faces[Model::Face::FACE_NORTH].direction == Model::Face::FACE_NORTH)
			vertex_count += 6;
		
		if(element->faces[Model::Face::FACE_EAST].direction == Model::Face::FACE_EAST)
			vertex_count += 6;
		
		if(element->faces[Model::Face::FACE_DOWN].direction == Model::Face::FACE_DOWN)
			vertex_count += 6;
	}
	
	return vertex_count;
}

CustomVertex ResourceManager::mapCustomVertex(const Model::Element *e, const VF3 &pt, const VF2 &uv, float tx_page)
{
	// TODO: properly map color here when needed.
	//  for tinting things like grass and leaves in different biomes.
	return CustomVertex(pt.f1, pt.f2, pt.f3, uv.f1, uv.f2, Color(), tx_page);
}

Model::Element::UV_MAP ResourceManager::mapFaceUV(const Model::Element *e, Model::Face::FaceDirection fdir, float &tx_page)
{
	const Model::Face &face = e->faces[fdir];
	auto uv_map = Model::Element::UV_MAP(face.uv);
	auto &uv = uv_map.uv;
	
	Atlas *atlas = getAtlas();
	
	Resource::ID tex_res = getBitmap(face.texname);
	if(tex_res != Resource::INVALID_ID)
	{
		Atlas::Item item;
		if(getAtlasItem(tex_res, &item))
		{
			float xfact = atlas->xfact();
			float yfact = atlas->yfact();
			
			uv.f1 = uv.f1 * xfact + item.x * xfact;
			uv.f2 = uv.f2 * yfact + item.y * yfact;
			
			uv.f3 = uv.f3 * xfact + item.x * xfact;
			uv.f4 = uv.f4 * yfact + item.y * yfact;
			
			tx_page = item.sheet + 1;
		}
	}
	
	if(uv.f1 == 0.0 && uv.f2 == 0.0 && uv.f3 == 0.0 && uv.f4 == 0.0)
	{
		uv.f1 = 0.0;
		uv.f2 = 0.0;
		uv.f3 = 1.0 * atlas->xfact();
		uv.f4 = 1.0 * atlas->yfact();
	}
	
	return uv_map;
}

bool ResourceManager::putModelVariant(Resource::ID id)
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

//rapidjson::Document *ResourceManager::getBlockstateJson(const std::string &name)
//{
//	return getJson(resPath(blockstatePath(name)));
//}

//rapidjson::Document *ResourceManager::getModelJson(const std::string& name)
//{
//	// cache ftw
//	std::string path = resPath(modelPath(name));
//	
//	return getJson(resPath(modelPath(name)));
//}

/*rapidjson::Document *ResourceManager::getJson(const std::string& p)
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
	
	if(al_fread(fh, buffer, fsize) != (size_t)fsize)
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
}*/

/*
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
*/
