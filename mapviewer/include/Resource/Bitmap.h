#ifndef RESOURCEBITMAP_H_GUARD
#define RESOURCEBITMAP_H_GUARD
#include <string>
#include "Resource.h"
#include "NBT_Debug.h"

class ResourceBitmap : public Resource
{
	public:
		ResourceBitmap(const std::string &path) : Resource(Resource::BitmapType, path) { NBT_Debug("new bmp[%i]: %s", id(), path.c_str()); }
		virtual ~ResourceBitmap() { }
		
	private:
};

#endif /* RESOURCEBITMAP_H_GUARD */
