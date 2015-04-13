#include "VBO.h"

#include <vector>
#include <allegro5/allegro_primitives.h>

#include "glm/vec3.hpp"
#include "glm/geometric.hpp"

#include "NBT_Debug.h"
#include "CustomVertex.h"
#include "Resource/Manager.h"

VBO::VBO()
{
	// nada
}


VBO::~VBO()
{
	// nada
}

void VBO::clear()
{
	m_data.clear();
}

void VBO::addLine(const glm::vec3& p1, const glm::vec3& p2, const Color& c1, float thickness)
{
	addLine(p1, p2, c1, c1, thickness);
}

void VBO::addLine(const glm::vec3& p1, const glm::vec3& p2, const Color& c1, const Color& c2, float thickness)
{
	float half_thickness = thickness * 0.5f;
	glm::vec3 diff = p2 - p1;
	float length = glm::distance(p2, p1);
	
	if(length == 0.0f)
		return;
	
	float tx = half_thickness * diff.y / length;
	float ty = half_thickness * -diff.x / length;
	
	//if(diff.x > diff.y) // horizontal
		addQuad(
			{ p1.x - tx, p1.y - ty, p1.z }, 
			{ p1.x + tx, p1.y + ty, p1.z },
			{ p2.x + tx, p2.y + ty, p2.z },
			{ p2.x - tx, p2.y - ty, p2.z },
			c1, c1, c2, c2
		);
	//else // vertical
	/*	addQuad(
			{ p1.x - half_thickness, p1.y, p1.z }, 
			{ p2.x - half_thickness, p2.y, p2.z },
			{ p2.x + half_thickness, p2.y, p2.z },
			{ p1.x + half_thickness, p1.y, p1.z }, 
			c1, c2, c2, c1
		);*/
}

void VBO::addQuad(const glm::vec3& p1, float width, float height, const Color& c1)
{
	addQuad(p1, width, height, c1, c1, c1, c1);
}

void VBO::addQuad(const glm::vec3& p1, float width, float height, const Color& c1, const Color& c2, const Color& c3, const Color& c4)
{
	glm::vec3 p2 = { p1.x,         p1.y + height, p1.z };
	glm::vec3 p3 = { p1.x + width, p1.y + height, p1.z };
	glm::vec3 p4 = { p1.x + width, p1.y,          p1.z };
	
	addQuad(p1, p2, p3, p4, c1, c2, c3, c4);
}

void VBO::addQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const Color& c1)
{
	addQuad(p1, p2, p3, p4, c1, c1, c1, c1);
}

// Assumes verticies are in proper order (ie: CCW for normal GL)
void VBO::addQuad(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const Color& c1, const Color& c2, const Color& c3, const Color& c4)
{
	addTriangle(p1, p2, p3, c1, c2, c3);
	addTriangle(p2, p3, p4, c2, c3, c4);
}

void VBO::addTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const Color& c1)
{
	addTriangle(p1, p2, p3, c1, c1, c1);
}

void VBO::addTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const Color& c1, const Color& c2, const Color& c3)
{
	m_data.emplace_back(CustomVertex(p1.x, p1.y, p1.z, 0.0f, 0.0f, c1, 0.0f));
	m_data.emplace_back(CustomVertex(p2.x, p2.y, p2.z, 0.0f, 0.0f, c2, 0.0f));
	m_data.emplace_back(CustomVertex(p3.x, p3.y, p3.z, 0.0f, 0.0f, c3, 0.0f));
}

void VBO::addWireCube(const glm::vec3& p1, const glm::vec3& p7, const Color& c, float thickness)
{
	glm::vec3 p2 = { p1.x, p7.y, p1.z },
				 p3 = { p7.x, p7.y, p1.z },
				 p4 = { p7.x, p1.y, p1.z },
				 p5 = { p1.x, p1.y, p7.z },
				 p6 = { p7.x, p1.y, p7.z },
				 p8 = { p1.x, p7.y, p7.z };
	
	addWireCube(p1, p2, p3, p4, p5, p6, p7, p8, c, thickness);
}

void VBO::addWireCube(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, 
							 const glm::vec3& p5, const glm::vec3& p6, const glm::vec3& p7, const glm::vec3& p8,
							 const Color& c, float thickness)
{
	// front face
	addLine(p1, p2, c, thickness);
	addLine(p2, p3, c, thickness);
	addLine(p3, p4, c, thickness);
	addLine(p4, p1, c, thickness);

	// back face
	addLine(p5, p6, c, thickness);
	addLine(p6, p7, c, thickness);
	addLine(p7, p8, c, thickness);
	addLine(p8, p5, c, thickness);
	
	// connect two faces
	addLine(p1, p5, c, thickness);
	addLine(p2, p8, c, thickness);
	addLine(p3, p7, c, thickness);
	addLine(p4, p6, c, thickness);
}


void VBO::draw(ResourceManager *rm)
{
	ALLEGRO_VERTEX_BUFFER *vbo = al_create_vertex_buffer(rm->vtx_decl(), m_data.data(), m_data.size(), ALLEGRO_PRIM_BUFFER_STATIC);
	if(!vbo)
	{
		NBT_Error("failed to create vbo :(");
		return;
	}
	
	al_draw_vertex_buffer(vbo, nullptr, 0, m_data.size(), ALLEGRO_PRIM_TRIANGLE_LIST);
	al_destroy_vertex_buffer(vbo);
}
