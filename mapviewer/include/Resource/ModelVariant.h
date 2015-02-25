#ifndef RESOURCMODELVARIANT_H_GUARD
#define RESOURCMODELVARIANT_H_GUARD
#include <string>
#include "Resource.h"
#include "NBT_Debug.h"

namespace Model {
	class Model;
}

class CustomVertex;

class ResourceModelVariant : public Resource
{
	public:
		ResourceModelVariant(const std::string &path, CustomVertex *vdata, uint32_t vcount);
		virtual ~ResourceModelVariant();
		
		uint32_t getVertexCount() { return m_vertex_count; }
		CustomVertex *getVertexData() { return m_vertexes; }
		
	private:
		uint32_t m_vertex_count;
		CustomVertex *m_vertexes;
};

#endif /* RESOURCMODELVARIANT_H_GUARD */
