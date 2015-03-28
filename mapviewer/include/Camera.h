#include <glm/glm.hpp>
#include "allegro5/transformations.h"

class Camera
{
	public:
		Camera();
		Camera(const glm::vec3 &p, const glm::vec3 &f, const glm::vec3 &u, float mspeed, float rotspeed);
		~Camera();
		
		void setPos(const glm::vec3 &p) { m_position = p; }
		void setForward(const glm::vec3 &t) { m_forward = t; }
		void setUp(const glm::vec3 &u) { m_up = u; }
		void setRight(const glm::vec3 &l) { m_right = l; }
		void setMovementSpeed(float ms) { m_movement_speed = ms; }
		void setRotationSpeed(float rs) { m_rotation_speed = rs; }
		
		void moveForward();
		void moveBack();
		void moveRight();
		void moveLeft();

		void moveUp();
		void moveDown();
		
		void look(float xdiff, float ydiff);
		
		void update();
		bool willUpdate() { return m_do_update; }
		
		const glm::vec3 &getPos() { return m_position; }
		const glm::vec3 &getTarget() { return m_forward; }
		const glm::vec3 &getUp() { return m_up; }
		
		const ALLEGRO_TRANSFORM &getMat() { return m_trans; }
		
		glm::vec3 getAdjPos(const glm::vec3 &to) { return m_position + to; }
		glm::vec3 getForward(float fd);
		
	private:
		glm::vec3 m_position;
		glm::vec3 m_forward;
		glm::vec3 m_up;
		glm::vec3 m_right;
		
		ALLEGRO_TRANSFORM m_trans;
		float m_movement_speed;
		float m_rotation_speed;
		bool m_do_update;
};
