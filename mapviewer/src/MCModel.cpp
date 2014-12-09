#include "MCModel.h"
#include "Resource/Manager.h"

#ifdef Bool
#undef Bool
#endif
#include "rapidjson/document.h"
#include "NBT_Debug.h"

// assuming [ x, z, y ]

// element from hopper:
// "from": [ 0, 11, 0 ],
// "to": [ 2, 16, 16 ],

// or from cube:
// "from": [ 0, 0, 0 ],
// "to": [ 16, 16, 16 ],
				
// how to turn the above into verticies...
/* first need to get the individual faces...
 * 
 * [ from.x, from.z, from.y ], [ from.x, from.z, to.y ], 
 * 
 * 
 */

MCModel::MCModel()
{ }

MCModel::~MCModel()
{ }

bool MCModel::loadVariant(const std::string &key, rapidjson::Value &v, ResourceManager *rm)
{
	Variant variant;
	
	NBT_Debug("load Variant: %s", key.c_str());
	if(!variant.load(key, v, rm))
		return false;
	
	variants_.push_back(variant);
	
	return true;
}

bool MCModel::loadBlockstate(const std::string &name, ResourceManager *rm)
{
	rapidjson::Document *doc = rm->getBlockstateJson(name);
	if(!doc)
		return false;
	
	NBT_Debug("load Blockstate: %s", name.c_str());
	
	if(doc->HasParseError())
	{
		NBT_Debug("json parse error");
		return false;
	}
	
	rapidjson::Value &variants = (*doc)["variants"];
	if(!variants.MemberCount())
	{
		NBT_Debug("no variants?");
		return false;
	}
	
	for(auto it = variants.MemberBegin(); it != variants.MemberEnd(); it++)
	{
		const char *member_key = it->name.GetString();
		rapidjson::Value &member_value = it->value;
		
		// value can be an array or object
		
		if(member_value.IsObject())
		{
			if(!loadVariant(member_key, member_value, rm))
				continue;
		}
		else if(member_value.IsArray())
		{
			int vid = 0;
			for(auto it = member_value.Begin(); it != member_value.End(); it++)
			{
				char buff[101];
				snprintf(buff, 100, "%i", vid);
				vid++;
				if(!loadVariant(buff, *it, rm))
					continue;
			}
		}
		else { NBT_Debug("unknown variant type?"); }
	}
	
	name_ = name;
	
	return true;
}

std::string MCModel::Variant::lookupTextureKey(const std::string &s)
{
	return texture_map_[s];
}

bool MCModel::Variant::loadElements(rapidjson::Value &v, ResourceManager *rm)
{
	if(v.IsNull() || !v.IsArray())
	{
		NBT_Debug("elements is not an array?");
		return false;
	}
	
	for(auto it = v.Begin(); it != v.End(); it++)
	{
		Element element;
		if(!element.load(this, *it, rm))
			return false;
		
		elements_.emplace(elements_.end(), element);
	}
	
	return true;
}

bool MCModel::Variant::loadTextures(rapidjson::Value &v)
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

bool MCModel::Variant::loadModel(const std::string &name, ResourceManager* rm)
{
	rapidjson::Document *doc = rm->getModelJson(name);
	if(!doc)
	{
		NBT_Debug("failed to get model json: %s", name.c_str());
		return false;
	}
	
	NBT_Debug("load Model: %s", name.c_str());
	
	if(doc->HasParseError())
	{
		NBT_Debug("json parse error?");
		return false;
	}
	
	const char *parent_name = nullptr;
	
	for(auto v = doc->MemberBegin(); v != doc->MemberEnd(); v++)
	{
		//NBT_Debug("member: %s:%i", v->name.GetString(), v->value.GetType());
		if(v->name == "elements")
		{
			if(!loadElements(v->value, rm))
			{
				NBT_Debug("failed to load elements :(");
				return false;
			}
		}
		else if(v->name == "textures")
		{
			if(!loadTextures(v->value))
			{
				NBT_Debug("failed to load textures :(");
				return false;
			}
		}
		else if(v->name == "parent")
		{
			//NBT_Debug("parent: %s", v->value.GetString());
			parent_name = v->value.GetString();
		}
	}

	// odly, we need to handle parent after the main content.
	//  references are made from parent resources to textures defined in child resources.
	if(parent_name)
	{
		//NBT_Debug("load parent %s", parent_name);
		if(!loadModel(parent_name, rm))
		{
			NBT_Debug("failed to load parent %s", parent_name);
			return false;
		}
		
	}
	
	return true;
}

MCModel *MCModel::Create(const std::string &name, ResourceManager *rm)
{
	MCModel *model = new MCModel;
	
	if(!model->loadBlockstate(name, rm))
	{
		delete model;
		return nullptr;
	}
	
	return model;
}

void MCModel::dump()
{
	NBT_Debug("dump model: %s", name_.c_str());
	for(auto it = variants_.begin(); it != variants_.end(); it++)
	{
		it->dump();
	}
}