#ifndef VBO_H_Guard
#define VBO_H_Guard

#include <vector>
#include "glm/fwd.hpp"
class CustomVertex;
class Color;
class ResourceManager;

class VBO
{
	public:
		VBO();
		~VBO();
		
		void addTriangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const Color &c1);
		void addTriangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const Color &c1, const Color &c2, const Color &c3);

		void addQuad(const glm::vec3 &p1, float width, float height, const Color &c1);
		void addQuad(const glm::vec3 &p1, float width, float height, const Color &c1, const Color &c2, const Color &c3, const Color &c4);
		void addQuad(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4, const Color &c1);
		void addQuad(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4, const Color &c1, const Color &c2, const Color &c3, const Color &c4);
		
		void addLine(const glm::vec3 &p1, const glm::vec3 &p2, const Color &c1, float thickness = 1.0f);
		void addLine(const glm::vec3 &p1, const glm::vec3 &p2, const Color &c1, const Color &c2, float thickness = 1.0f);

		void addWireCube(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4,
							  const glm::vec3 &p5, const glm::vec3 &p6, const glm::vec3 &p7, const glm::vec3 &p8, const Color &c, float thickness = 1.0f);
		
		void addWireCube(const glm::vec3 &p1, const glm::vec3 &p2, const Color &c, float thickness = 1.0f);
		
		void draw(ResourceManager *rm);
		void clear();
		
	private:
		std::vector<CustomVertex> m_data;
		ResourceManager *m_resManager;
};

#endif /* VBO_H_Guard */
