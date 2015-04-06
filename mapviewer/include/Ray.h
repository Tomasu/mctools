#ifndef Ray_H_GUARD
#define Ray_H_GUARD
#include <glm/detail/type_vec3.hpp>
#include <glm/geometric.hpp>

class Ray
{
	public:
		Ray(const glm::vec3 &start, const glm::vec3 &end) : m_start(start), m_end(end), m_dir(glm::normalize(end-start)) { }
		Ray(const glm::vec3 &start, const glm::vec3 &dir, float length) : m_start(start), m_end(start + dir * length), m_dir(glm::normalize(dir)) { }
		~Ray() { }
		
		glm::vec3 start() const { return m_start; }
		glm::vec3 end() const { return m_end; }
		float length() const { return glm::distance(m_end, m_start); }
		glm::vec3 direction() const { return m_dir; }
		
	private:
		glm::vec3 m_start;
		glm::vec3 m_end;
		glm::vec3 m_dir;
};

#endif /* Ray_H_GUARD */
