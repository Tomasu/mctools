#include "MCModel.h"
#include "Resource/Manager.h"

#ifdef Bool
#undef Bool
#endif
#include "rapidjson/document.h"
#include "NBT_Debug.h"

MCModel::MCModel() : ambientocclusion_(false)
{ }

MCModel::~MCModel()
{ }

std::string MCModel::lookupTextureKey(const std::string &s)
{
	return texture_map_[s];
}

bool MCModel::loadElements(rapidjson::Value &v)
{
	if(v.IsNull() || !v.IsArray())
	{
		NBT_Debug("elements is not an array?");
		return false;
	}
	
	for(auto it = v.Begin(); it != v.End(); it++)
	{
		Element element;
		if(!element.load(this, *it))
			return false;
		
		elements_.emplace(elements_.end(), element);
	}
	
	return true;
}

bool MCModel::loadTextures(rapidjson::Value &v)
{
	if(v.IsNull() || !v.IsObject())
		return false;
	
	for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
	{
		const char *tex_key = it->name.GetString();
		const char *val_name = it->value.GetString();
		
		const char *tex_name = val_name[0] != '#' ? val_name : lookupTextureKey(&(val_name[1])).c_str();
		
		texture_map_.emplace(tex_key, tex_name);
	}
	
	return true;
}

bool MCModel::loadTags(const std::string &name, ResourceManager* rm)
{
	rapidjson::Document *doc = rm->getJson(name);
	if(!doc)
		return false;
	
	if(doc->HasParseError())
		return false;
	
	rapidjson::Value elements, textures, parent_val;
	
	for(auto v = doc->MemberBegin(); v != doc->MemberEnd(); v++)
	{
		if(v->name == "elements")
			elements = v->value;
		else if(v->name == "textures")
			textures = v->value;
		else if(v->name == "parent")
			parent_val = v->value;
	}
	
	if(!textures.IsNull())
	{
		if(!loadTextures(textures))
		{
			NBT_Debug("failed to load textures :(");
			return false;
		}
	}
	
	// elements reference textures
	if(!elements.IsNull())
	{
		if(!loadElements(elements))
		{
			NBT_Debug("failed to load elements :(");
			return false;
		}
	}
	
	
	// odly, we need to handle parent after the main content.
	//  references are made from parent resources to textures defined in child resources.
	if(!parent_val.IsNull())
	{
		if(!loadTags(parent_val.GetString(), rm))
		{
			NBT_Debug("failed to load parent %s", parent_val.GetString());
			return false;
		}
		
	}
	
	return true;
}

MCModel *MCModel::Create(const std::string &name, ResourceManager *rm)
{
	MCModel *model = new MCModel;
	
	if(!model->loadTags(name, rm))
	{
		delete model;
		return nullptr;
	}
	
	return model;
}

void MCModel::dump()
{
	NBT_Debug("textures:");
	for(auto it: texture_map_)
	{
		NBT_Debug("%s => %s", it.first.c_str(), it.second.c_str());
	}
	
	NBT_Debug("elements:");
	for(auto &element: elements_)
	{
		NBT_Debug("from: [%f,%f,%f] to: [%f,%f,%f]", element.from.f1, element.from.f2, element.from.f3, element.to.f1, element.to.f2, element.to.f3);
		
		for(int i = 0; i < Face::MAX_FACES; i++)
		{
			Face &face = element.faces[i];
			
			NBT_Debug("face[%i]: uv:[%f,%f,%f,%f] texture:[%s] cull:[%i] tintindex:[%i]",
						 i, face.uv.f1, face.uv.f2, face.uv.f3, face.uv.f4, face.texname.c_str(), face.cull, face.tintindex);
		}
	}
}
