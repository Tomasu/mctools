#ifndef RESOURCEMANAGER_H_GUARD
#define RESOURCEMANAGER_H_GUARD
#include <unordered_map>

#include "Atlas.h"
#include "Resource/Resource.h"

class Renderer;
class ALLEGRO_BITMAP;
class ResourceBitmap;

class ResourceManager
{
	public:
		
		ResourceManager(Renderer *renderer, const std::string &basePath = std::string("assets/minecraft"), const std::string &bmpSubPath = std::string("textures"));
		~ResourceManager();
		
		bool getAtlasItem(Resource::ID id, Atlas::Item *item);
		
		Resource::ID getBitmap(const std::string &name);
		Resource::ID refBitmap(const std::string &name);
		bool putBitmap(Resource::ID id);
		
		// bitmapFromID?
		
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
		
		
	private:
		std::string baseResourcePath_;
		std::string bitmapSubPath_;
		
		std::unordered_map<Resource::ID, Resource *> resources_;
		std::unordered_map<std::string, Resource::ID> nameToIDMap_;
		
		// TODO: maybe add a field to store the time resources were put in the unload list
		//  then 'cleanup' can treat this is a lru list, and only unload after a certain time has gone by?
		std::unordered_map<std::string, Resource *> resToUnload_;
		
		Atlas *atlas_;
		
		Renderer *renderer_;
		
		std::string resPath(const std::string &name);
		std::string bmpPath(const std::string &name);
		
		Resource::ID findID(const std::string &path);
		
		Resource *findResource(Resource::ID id);
		bool putResource(Resource::ID id);
};

#endif /* RESOURCEMANAGER_H_GUARD */
