#ifndef MCMODEL_H_GUARD
#define MCMODEL_H_GUARD

#include <unordered_map>
#include <map>
#include <cmath>
#include "Resource/Manager.h"
#include "CustomVertex.h"
#include "Util.h"
#include "NBT_Debug.h"

void al_transform_coordinates_3d(const ALLEGRO_TRANSFORM *trans, float *x, float *y, float *z);

class ResourceManager;

class MCModel
{
	public:
		struct Coord3f {
			float f1, f2, f3;
			
			Coord3f(float f1_ = 0.0, float f2_ = 0.0, float f3_ = 0.0) : f1(f1_), f2(f2_), f3(f3_) { }
			
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
			
			Coord2f(float f1_ = 0.0, float f2_ = 0.0) : f1(f1_), f2(f2_) { }
			
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
			
			Coord4f(float f1_ = 0.0, float f2_ = 0.0, float f3_ = 0.0, float f4_ = 0.0) : f1(f1_), f2(f2_), f3(f3_), f4(f4_) { }
			
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
		
		struct Variant;
		
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
				FACE_NONE = -1,
				FACE_UP = 0,
				FACE_DOWN,
				FACE_NORTH,
				FACE_EAST,
				FACE_SOUTH,
				FACE_WEST
			};
			
			FaceDirection direction;
			Coord4f uv;
			std::string texname;
			Resource::ID tex_res;
			uint32_t tex_page;
			CullFace cull;
			int32_t tintindex;
			float rotation;
			
			Face() : direction(FACE_NONE), uv(), texname(), cull(CULL_NONE), tintindex(-1) { }
			// TODO: putBitmap for texture?
			
			bool load(Variant *variant, FaceDirection dir, rapidjson::Value &v, ResourceManager *rm)
			{
				if(v.IsNull() || !v.IsObject())
					return false;
				
				direction = dir;
				rotation = 0.0;
				
				uv.f1 = 0.0;
				uv.f2 = 0.0;
				uv.f3 = 1.0;
				uv.f4 = 1.0;
					
				Atlas *atlas = rm->getAtlas();
				
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
						const char *tex_name = tex_key[0] != '#' ? tex_key : variant->lookupTextureKey(&(tex_key[1])).c_str();
						
						//NBT_Debug("face tex: %s -> %s", tex_key, tex_name);
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
					else if(it->name == "rotation")
					{
						//rotation = -it->value.GetDouble();
					}
				}
				
				tex_res = rm->getBitmap(texname);
				if(tex_res != Resource::INVALID_ID)
				{
					Atlas::Item item;
					if(rm->getAtlasItem(tex_res, &item))
					{
						float xfact = atlas->xfact();
						float yfact = atlas->yfact();
						
						uv.f1 = uv.f1 * xfact + item.x * xfact;
						uv.f2 = uv.f2 * yfact + item.y * yfact;
						
						uv.f3 = uv.f3 * xfact + item.x * xfact;
						uv.f4 = uv.f4 * yfact + item.y * yfact;
						
						tex_page = item.sheet + 1;
					}
				}
				
				if(uv.f1 == 0.0 && uv.f2 == 0.0 && uv.f3 == 0.0 && uv.f4 == 0.0)
				{
					uv.f1 = 0.0;
					uv.f2 = 0.0;
					uv.f3 = 1.0 * atlas->xfact();
					uv.f4 = 1.0 * atlas->yfact();
				}
				
				if(rotation != 0.0)
				{
					ALLEGRO_TRANSFORM rot;
					al_identity_transform(&rot);
					al_translate_transform(&rot, -0.5, -0.5);
					al_rotate_transform(&rot, rotation * M_PI / 180.0);
					al_translate_transform(&rot, 0.5, 0.5);
					
					al_transform_coordinates(&rot, &uv.f1, &uv.f2);
					al_transform_coordinates(&rot, &uv.f3, &uv.f4);
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
			private:
				struct UV_MAP
				{
					UV_MAP() : uv() { }
					
					UV_MAP(const Coord4f &uvd) : uv(uvd) { }
					
					VF2 p3() { return VF2(uv.f1, uv.f2); }
					VF2 p4() { return VF2(uv.f3, uv.f2); }
					VF2 p1() { return VF2(uv.f1, uv.f4); }
					VF2 p2() { return VF2(uv.f3, uv.f4); }
					
					Coord4f uv;
				};
				
				struct POINT_MAP
				{
					POINT_MAP() { }
					POINT_MAP(const Coord3f &from, const Coord3f &to) : from_(from), to_(to)
					{ }
					
					VF3 from1() { return VF3(from_.f1, from_.f2, from_.f3); }
					VF3 from2() { return VF3(to_.f1,   from_.f2, from_.f3); }
					VF3 from3() { return VF3(from_.f1, to_.f2,   from_.f3); }
					VF3 from4() { return VF3(to_.f1,   to_.f2,   from_.f3); }
					
					VF3 to1() { return VF3(from_.f1, from_.f2, to_.f3); }
					VF3 to2() { return VF3(to_.f1,   from_.f2, to_.f3); }
					VF3 to3() { return VF3(from_.f1, to_.f2,  to_.f3); }
					VF3 to4() { return VF3(to_.f1,   to_.f2,  to_.f3); }
					
					Coord3f from_, to_;
				};
				
			public:
				Coord3f from, to;
				Rotation rotation;
				bool shade;
				Face faces[Face::MAX_FACES];
				
				uint32_t vertex_count;
				uint32_t vidx;
				CUSTOM_VERTEX *vertices;
				
				bool loadFaces(Variant *variant, rapidjson::Value &v, ResourceManager *rm)
				{
					if(v.IsNull() || !v.IsObject())
					{
						NBT_Debug("faces is null or not an object?");
						return false;
					}
					
					int32_t face_count = 0;
					for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
					{
						if(it->name == "up")
						{
							if(!faces[Face::FACE_UP].load(variant, Face::FACE_UP, it->value, rm))
								return false;
							
							face_count++;
						}
						else if(it->name == "down")
						{
							if(!faces[Face::FACE_DOWN].load(variant, Face::FACE_DOWN, it->value, rm))
								return false;
							
							face_count++;
						}
						else if(it->name == "north")
						{
							if(!faces[Face::FACE_NORTH].load(variant, Face::FACE_NORTH, it->value, rm))
								return false;
							
							face_count++;
						}
						else if(it->name == "east")
						{
							if(!faces[Face::FACE_EAST].load(variant, Face::FACE_EAST, it->value, rm))
								return false;
							
							face_count++;
						}
						else if(it->name == "south")
						{
							if(!faces[Face::FACE_SOUTH].load(variant, Face::FACE_SOUTH, it->value, rm))
								return false;
							
							face_count++;
						}
						else if(it->name == "west")
						{
							if(!faces[Face::FACE_WEST].load(variant, Face::FACE_WEST, it->value, rm))
								return false;
							
							face_count++;
						}
					}
					
					vertex_count = face_count * 6;
					vertices = new CUSTOM_VERTEX[vertex_count];
					vidx = 0;
					
					POINT_MAP pmap_ = POINT_MAP(from, to);
					
					NBT_Debug("got %i faces, need %i vertices", face_count, vertex_count);
					
					if(faces[Face::FACE_UP].direction == Face::FACE_UP)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_UP].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to3(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from4(), uv.p2());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from3(), uv.p1());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to3(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to4(), uv.p4());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from4(), uv.p2());
					}
					
					if(faces[Face::FACE_SOUTH].direction == Face::FACE_SOUTH)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_SOUTH].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to2(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to4(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p2());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p2());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to4(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to3(), uv.p4());
					}
					
					if(faces[Face::FACE_WEST].direction == Face::FACE_WEST)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_WEST].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to4(), uv.p4());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to2(), uv.p2());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from4(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to4(), uv.p4());
					}
					
					if(faces[Face::FACE_NORTH].direction == Face::FACE_NORTH)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_NORTH].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from1(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from3(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p2());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p2());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from3(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from4(), uv.p4());
					}
					
					if(faces[Face::FACE_EAST].direction == Face::FACE_EAST)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_EAST].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from1(), uv.p2());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from3(), uv.p4());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to3(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from3(), uv.p4());
					}
					
					if(faces[Face::FACE_DOWN].direction == Face::FACE_DOWN)
					{
						UV_MAP uv = UV_MAP(faces[Face::FACE_DOWN].uv);
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from1(), uv.p3());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p4());
						
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to1(), uv.p1());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.from2(), uv.p4());
						vertices[vidx++] = CUSTOM_VERTEX(pmap_.to2(), uv.p2());
					}
					
					NBT_Debug("wanted verts: %i, got %i", vertex_count, vidx);
					
					return true;
				}
				
				bool load(Variant *variant, rapidjson::Value &v, ResourceManager *rm)
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
							if(!loadFaces(variant, it->value, rm))
							{
								NBT_Debug("failed to load faces");
								return false;
							}
						}
					}
					
					return true;
				}
		};
		
		struct Variant
		{
			std::string key;
			std::string model;
			int x, y;
			bool uvlock;
			bool ambientocclusion;
			
			std::map<std::string, std::string> texture_map_;
			std::vector<Element> elements_;
			std::map<std::string, std::string> state_map_;
			
			
			
			bool load(const std::string &k, rapidjson::Value &v, ResourceManager *rm)
			{
				if(v.IsNull() || !v.IsObject())
				{
					NBT_Debug("Variant is not valid?");
					return false;
				}
				
				x = 0.0;
				y = 0.0;
				key = k;
				
				size_t last_pos = -1, pos = 0;
				std::string bstate_key;
				while((pos = key.find_first_of(",=", last_pos+1)) != std::string::npos)
				{
					std::string part = key.substr(last_pos+1, pos - last_pos - 1 );
					
					if(key[pos] == '=')
					{
						bstate_key = part;
					}
					else if(key[pos] == ',')
					{
						state_map_.emplace(bstate_key, part);
						NBT_Debug("bstate: %s=%s", bstate_key.c_str(), part.c_str());
						bstate_key.erase();
					}
					
					last_pos = pos;
				}
				
				if(!bstate_key.empty())
				{
					std::string part = key.substr(last_pos+1);
					NBT_Debug("bstate: %s=%s", bstate_key.c_str(), part.c_str());
					state_map_.emplace(bstate_key, part);
				}
				
				for(auto it = v.MemberBegin(); it != v.MemberEnd(); it++)
				{
					if(it->name == "model")
						model = it->value.GetString();
					else if(it->name == "x")
						x = it->value.GetInt();
					else if(it->name == "y")
						y = it->value.GetInt();
					else if(it->name == "uvlock")
						uvlock = it->value.GetBool();
				}
				
				if(!model.length())
				{
					NBT_Debug("variant is missing name property");
					return false;
				}
				
				model = "block/" + model;
				
				if(!loadModel(model, rm))
				{
					NBT_Debug("failed to load variant model?");
					return false;
				}
				
				rotate();
				
				return true;
			}
			
			bool loadModel(const std::string &name, ResourceManager *rm);
			bool loadElements(rapidjson::Value &v, ResourceManager *rm);
			bool loadTextures(rapidjson::Value &v);
			
			std::string lookupTextureKey(const std::string &s);
			
			void rotate()
			{
				if(x == 0.0 && y == 0.0)
					return;
				
				ALLEGRO_TRANSFORM rot;
				al_identity_transform(&rot);
				al_translate_transform_3d(&rot, -0.5, -0.5, -0.5);
				
				if(x != 0.0)
				{
				//	al_translate_transform_3d(&xrot, -1.0, -1.0, -1.0);
					al_rotate_transform_3d(&rot, 1, 0, 0, x * M_PI / 180.0);
					//al_translate_transform_3d(&xrot, -1.0, 0.0, 0.0);
				}
				
				if(y != 0.0)
				{
					//al_translate_transform_3d(&yrot, 0.0, 1.0, 0.0);
					al_rotate_transform_3d(&rot, 0, 1, 0, y * M_PI / 180.0);
					//al_translate_transform_3d(&yrot, 0.0, -1.0, 0.0);
				}
				
				//al_compose_transform(&xrot, &yrot);
				al_translate_transform_3d(&rot, 0.5, 0.5, 0.5);
				
				for(auto &elem: elements_)
				{
					for(int32_t i = 0; i < elem.vertex_count; i++)
					{
						CUSTOM_VERTEX &v = elem.vertices[i];
						al_transform_coordinates_3d(&rot, &(v.pos.f1), &(v.pos.f2), &(v.pos.f3));
					}
				}
			}
			
			void dump()
			{
				NBT_Debug("variant: %s::%s", model.c_str(), key.c_str());
				NBT_Debug("textures:");
				for(auto it: texture_map_)
				{
					NBT_Debug("%s => %s", it.first.c_str(), it.second.c_str());
				}
				
				NBT_Debug("elements:");
				for(auto &element: elements_)
				{
					NBT_Debug("from: [%f,%f,%f] to: [%f,%f,%f]", element.from.f1, element.from.f2, element.from.f3, element.to.f1, element.to.f2, element.to.f3);
					
					for(uint32_t i = 0; i < Face::MAX_FACES; i++)
					{
						Face &face = element.faces[i];
						
						NBT_Debug("face[%i]: uv:[%f,%f,%f,%f] texture:[%s] cull:[%i] tintindex:[%i]",
									i, face.uv.f1, face.uv.f2, face.uv.f3, face.uv.f4, face.texname.c_str(), face.cull, face.tintindex);
					}
				}
			}
		};
		
		MCModel();
		~MCModel();
		
		static MCModel *Create(const std::string &name, ResourceManager *rm);

		const std::vector<Variant> &getVariants() { return variants_; }
		
		void dump();
		
	private:
		std::string name_;
		std::vector<Variant> variants_;
		
		bool loadBlockstate(const std::string &name, ResourceManager *rm);
		bool loadVariant(const std::string &key, rapidjson::Value &v, ResourceManager *rm);
		
};

#endif /* MCMODEL_H_GUARD */
