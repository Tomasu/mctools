#ifndef Plane_H_GUARD
#define Plane_H_GUARD

#include "glm/detail/type_vec3.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/intersect.hpp"
#include "Ray.h"

class Plane
{
	public:
		Plane() { }
		~Plane() { }

		Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
		{
			glm::vec3 edge1 = p2 - p1;
			glm::vec3 edge2 = p3 - p1;

			m_normal = glm::normalize(glm::cross(edge1, edge2));
			m_origin = p1;
			m_distance = -glm::normalize(glm::dot( m_normal, p1 ));
		}
		
		Plane(const glm::vec3 &o, const glm::vec3 &n) : m_origin(o), m_normal(n) { }
		
		glm::vec3 origin() const { return m_origin; }
		glm::vec3 normal() const { return m_normal; }
		float distance() const { return m_distance; }
		
		bool intersect(const Ray &r, float &d)
		{
			return glm::intersectRayPlane(r.start(), r.direction(), m_origin, m_normal, d);
		}
		
	private:
		glm::vec3 m_origin;
		float m_distance;
		glm::vec3 m_normal;
};

#endif /* Plane_H_GUARD */
