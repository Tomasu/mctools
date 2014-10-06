#ifndef MCMODEL_H_GUARD
#define MCMODEL_H_GUARD

#include <unordered_map>
#include <map>
#include "Resource/Manager.h"
#include "Util.h"
#include "NBT_Debug.h"

class ResourceManager;
class MCModel;

class MCModel
{
	public:
		struct Coord3f {
			float f1, f2, f3;
			
			bool load(rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsArray())
					return false;
				
				f1 = v[0u].GetDouble() / 16.0;
				f2 = v[1u].GetDouble() / 16.0;
				f3 = v[2u].GetDouble() / 16.0;
				
				return true;
			}
		};
		
		struct Coord2f {
			float f1, f2;
			
			bool load(rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsArray())
					return false;
				
				f1 = v[0u].GetDouble() / 16.0;
				f2 = v[1u].GetDouble() / 16.0;
				
				return true;
			}
		};
		
		struct Coord4f {
			float f1, f2, f3, f4;
			
			bool load(rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsArray())
					return false;
				
				f1 = v[0u].GetDouble() / 16.0;
				f2 = v[1u].GetDouble() / 16.0;
				f3 = v[2u].GetDouble() / 16.0;
				f4 = v[3u].GetDouble() / 16.0;
				
				return true;
			}
		};
		
		struct Face {
			const static uint32_t MAX_FACES = 6;
			enum CullFace {
				CULL_NONE = 0,
				CULL_UP,
				CULL_DOWN,
				CULL_NORTH,
				CULL_EAST,
				CULL_SOUTH,
				CULL_WEST
			};
			
			enum FaceDirection {
				FACE_UP = 0,
				FACE_DOWN,
				FACE_NORTH,
				FACE_EAST,
				FACE_SOUTH,
				FACE_WEST
			};
			
			Coord4f uv;
			std::string texname;
			CullFace cull;
			uint32_t tintindex;
			
			bool load(MCModel *model, rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsObject())
					return false;
				
				uv.f1 = 0.0;
				uv.f2 = 0.0;
				uv.f3 = 1.0;
				uv.f4 = 1.0;
				
				cull = CULL_NONE;
				
				tintindex = 0;
				
				for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
				{
					if(it->name == "uv")
					{
						if(!uv.load(it->value))
							return false;
					}
					else if(it->name == "texture")
					{
						const char *tex_key = it->value.GetString();
						const char *tex_name = tex_key[0] != '#' ? tex_key : model->lookupTextureKey(&(tex_key[1])).c_str();
						
						NBT_Debug("face tex: %s -> %s", tex_key, tex_name);
						texname = std::string(tex_name);
					}
					else if(it->name == "cullface")
					{
						rapidjson::Value &v = it->value;
						
						if(v == "up")
							cull = CULL_UP;
						else if(v == "down")
							cull = CULL_DOWN;
						else if(v == "north")
							cull = CULL_NORTH;
						else if(v == "east")
							cull = CULL_EAST;
						else if(v == "south")
							cull = CULL_SOUTH;
						else if(v == "west")
							cull = CULL_WEST;
						else
							cull = CULL_NONE;
					}
					else if(it->name == "tintindex")
					{
						tintindex = it->value.GetInt();
					}
				}
				
				return true;
			}
		};
		
		struct Rotation {
			enum Axis {
				AXIS_NONE = 0,
				AXIS_Y,
				AXIS_Z, 
				AXIS_X
			};
			//"origin": [ 8, 8, 8 ], "axis": "y", "angle": 45, "rescale": true
			Coord3f origin;
			Axis axis;
			float angle;
			bool rescale;
			
			bool load(rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsObject())
					return false;
				
				for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
				{
					if(it->name == "origin")
					{
						if(!origin.load(it->value))
							return false;
					}
					else if(it->name == "axis")
					{
						rapidjson::Value &v = it->value;
						
						if(v.IsNull() || !v.IsString())
							return false;
						
						if(v == "x")
							axis = AXIS_X;
						else if(v == "y")
							axis = AXIS_Y;
						else if(v == "z")
							axis = AXIS_Z;
						else
							axis = AXIS_NONE;
					}
					else if(it->name == "angle")
					{
						double angledeg = it->value.GetDouble();
						angle = DEG_TO_RAD(angledeg);
					}
					else if(it->name == "rescale")
					{
						rescale = it->value.GetBool();
					}
				}
				
				return true;
			}
		};
		
		struct Element {
			Coord3f from, to;
			Rotation rotation;
			bool shade;
			Face faces[Face::MAX_FACES];
			
			bool loadFaces(MCModel *model, rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsObject())
				{
					NBT_Debug("faces is null or not an object?");
					return false;
				}
				
				for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
				{
					if(it->name == "up")
					{
						if(!faces[Face::FACE_UP].load(model, it->value))
							return false;
					}
					else if(it->name == "down")
					{
						if(!faces[Face::FACE_DOWN].load(model, it->value))
							return false;
					}
					else if(it->name == "north")
					{
						if(!faces[Face::FACE_NORTH].load(model, it->value))
							return false;
					}
					else if(it->name == "east")
					{
						if(!faces[Face::FACE_EAST].load(model, it->value))
							return false;
					}
					else if(it->name == "south")
					{
						if(!faces[Face::FACE_SOUTH].load(model, it->value))
							return false;
					}
					else if(it->name == "west")
					{
						if(!faces[Face::FACE_WEST].load(model, it->value))
							return false;
					}
				}
				
				return true;
			}
			
			bool load(MCModel *model, rapidjson::Value &v)
			{
				if(v.IsNull() || !v.IsObject())
				{
					NBT_Debug("Element is not valid?");
					return false;
				}
				
				for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
				{
					if(it->name == "from")
					{
						if(!from.load(it->value))
							return false;
					}
					else if(it->name == "to")
					{
						if(!to.load(it->value))
							return false;
					}
					else if(it->name == "rotation")
					{
						if(!rotation.load(it->value))
							return false;
					}
					else if(it->name == "shade")
					{
						shade = it->value.GetBool();
					}
					else if(it->name == "faces")
					{
						if(!loadFaces(model, it->value))
						{
							NBT_Debug("failed to load faces");
							return false;
						}
					}
				}
				
				return true;
			}
		};
		
		MCModel();
		~MCModel();
		
		static MCModel *Create(const std::string &name, ResourceManager *rm);

		void dump();
		
	private:
		bool ambientocclusion_;
		std::map<std::string, std::string> texture_map_;
		std::vector<Element> elements_;
		
		bool loadTags(const std::string &name, ResourceManager *rm);
		bool loadElements(rapidjson::Value &v);
		bool loadTextures(rapidjson::Value &v);
		
		std::string lookupTextureKey(const std::string &s);
};

#endif /* MCMODEL_H_GUARD */
