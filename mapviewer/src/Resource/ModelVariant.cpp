#include "Resource/ModelVariant.h"

#include "CustomVertex.h"

ResourceModelVariant::ResourceModelVariant(const std::string& path, CustomVertex* vdata, uint32_t vcount) 
	: Resource(Resource::ModelVariantType, path), m_vertex_count(vcount), m_vertexes(vdata) 
{ 
	NBT_Debug("new model variant[%i]: %s", id(), path.c_str());
}

ResourceModelVariant::~ResourceModelVariant()
{
	delete[] m_vertexes;
}
