#ifndef RESOURCEMANAGER_H_GUARD
#define RESOURCEMANAGER_H_GUARD
#include <unordered_map>

typedef struct ALLEGRO_PATH ALLEGRO_PATH;
#include "Atlas.h"
#include "Resource/Resource.h"

#ifdef Bool
#undef Bool
#endif

//#include "rapidjson/document.h"

class Renderer;
class ALLEGRO_BITMAP;
class ResourceBitmap;
class ResourceModel;
class ResourceModelVariant;
class BlockInfo;
class BlockAddress;
class Minecraft;

namespace Model {
	struct Variant;
}

struct ALLEGRO_VERTEX_BUFFER;
struct ALLEGRO_VERTEX_DECL;

#include "Model/Face.h"
#include "Model/Element.h"
#include "CustomVertex.h"

class ResourceManager
{
	public:
		
		ResourceManager(Renderer *renderer, const std::string &basePath = std::string("assets/minecraft"), const std::string &bmpSubPath = std::string("textures"), const std::string &modelSubPath = std::string("models"), const std::string &blockstateSubPath = std::string("blockstates"));
		~ResourceManager();
		
		bool init(Minecraft *mc, const char *argv0);
		
		bool getAtlasItem(Resource::ID id, Atlas::Item *item);
		
		Resource::ID getBitmap(const std::string &name);
		Resource::ID getLoadedBitmap(const std::string &name, bool fullpath = false);
		
		bool putBitmap(Resource::ID id);
		
		// bitmapFromID?
		
		Resource::ID getModel(const std::string &name);
		bool putModel(Resource::ID id);
		ResourceModel *getModelResource(Resource::ID id);
		
		Resource::ID getModelVariant(const BlockInfo &addr);
		bool putModelVariant(Resource::ID id);
		ResourceModelVariant *getModelVariantResource(Resource::ID id);
		
		Resource::ID getResource(Resource::Type type, const std::string &name);
		Resource::ID refResource(Resource::Type type, const std::string &name);

		bool refResource(Resource::ID id);
		
		bool resourceIsPinned(Resource::ID id);
		void pinResource(Resource::ID id);
		void unpinResource(Resource::ID id);
		
		// resFromID?
		
		bool resourceExists(Resource::ID id);
		bool resourceExists(Resource::Type type, const std::string &name);
		
		bool cleanup(uint32_t max);
		
		Atlas *getAtlas() { return atlas_; }
		bool setAtlasUniforms();
		void unsetAtlasUniforms();
		
		//rapidjson::Document *getModelJson(const std::string &name);
		//rapidjson::Document *getBlockstateJson(const std::string &name);
		//rapidjson::Document *getJson(const std::string &name);
		
		//ALLEGRO_PATH *locateMinecraftData();
		//ALLEGRO_PATH *locateMinecraftJar(ALLEGRO_PATH *mc_path);
		
		bool createMissingBlockBitmap();
		
		const char *getBlockStateName(const BlockAddress &baddr);
		
		ALLEGRO_VERTEX_BUFFER *sel_vbo() { return sel_vbo_; }
		ALLEGRO_VERTEX_DECL *vtx_decl() { return vtxdecl_; }
		
	private:
		std::string baseResourcePath_;
		std::string bitmapSubPath_;
		std::string modelSubPath_;
		std::string blockstateSubPath_;
		
		std::unordered_map<Resource::ID, Resource *> resources_;
		std::unordered_map<std::string, Resource::ID> nameToIDMap_;
		
		// TODO: maybe add a field to store the time resources were put in the unload list
		//  then 'cleanup' can treat this is a lru list, and only unload after a certain time has gone by?
		std::unordered_map<std::string, Resource *> resToUnload_;
		
		// json doc cache
		
		//std::unordered_map<std::string, rapidjson::Document *> jsonDocCache_;
		
		Atlas *atlas_;
		
		Renderer *renderer_;
		
		ALLEGRO_VERTEX_DECL *vtxdecl_;
		ALLEGRO_VERTEX_BUFFER *sel_vbo_;
		
		std::string resPath(const std::string &name);
		std::string bmpPath(const std::string &name);
		std::string modelPath(const std::string &name);
		std::string blockstatePath(const std::string &name);
		std::string modelVariantPath(const std::string &name, uint32_t variant);
		
		Resource::ID findID(const std::string &path);
		
		Resource *findResource(Resource::ID id);
		bool putResource(Resource::ID id);
		
		int32_t measureVariant(Model::Variant *v);
		CustomVertex mapCustomVertex(const Model::Element *e, const VF3 &pt, const VF2 &uv, float tx_page);
		Model::Element::UV_MAP mapFaceUV(const Model::Element *e, Model::Face::FaceDirection fdir, float &tx_page);
};

class MCVersion
{
	public:
		MCVersion() : str_(), time_(0), snapshot_(true) { }
		
		MCVersion(const std::string &str, time_t releaseTime, bool snapshot = false) : str_(str), time_(releaseTime), snapshot_(snapshot) { }
		~MCVersion() { }
		
		bool operator==(const MCVersion &in)
		{
			return (time_ == in.time_);
		}

		bool operator<(const MCVersion &in)
		{
			// short circuit on type, snap, alpha, beta, release, etc
			return (time_ < in.time_);
		}
		
		MCVersion &operator=(const MCVersion &rhs)
		{
			str_ = rhs.str_;
			time_ = rhs.time_;
			snapshot_ = rhs.snapshot_;
			
			return *this;
		}
		
		bool isValid() const { return time_ != 0; }
		bool isSnapshot() const { return snapshot_; }
		
		const std::string &str() const { return str_; }
		
	private:
		std::string str_;
		time_t time_;
		bool snapshot_;
};

#endif /* RESOURCEMANAGER_H_GUARD */
