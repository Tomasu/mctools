#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "NBT_Debug.h"

Camera::Camera()
{
	NBT_Debug("crap camera ctor");
}

Camera::Camera(const glm::vec3& p, const glm::vec3& f, const glm::vec3& u, float mspeed, float rotspeed)
	: m_position(p), m_forward(glm::normalize(f)), m_up(glm::normalize(u)), m_left(glm::normalize(glm::cross(u, f))), m_mat(1.0f), m_movement_speed(mspeed), m_rotation_speed(rotspeed), 
	  m_do_update(true)
{
	NBT_Debug("main camera ctor");
}

Camera::~Camera()
{

}

void Camera::look(float xdiff, float ydiff)
{
	NBT_Debug("look: %.02f,%.02f, rs:%.02f", xdiff,ydiff, m_rotation_speed);
	
	m_forward = glm::normalize(glm::rotate(m_forward, xdiff * m_rotation_speed, m_up));
	//m_left = glm::cross(m_up, m_forward);
	m_left = glm::normalize(glm::rotate(m_left, xdiff * m_rotation_speed, m_up));
	
	m_forward = glm::normalize(glm::rotate(m_forward, ydiff * m_rotation_speed, m_left));
	m_up = glm::normalize(glm::rotate(m_up, ydiff * m_rotation_speed, m_left));
	
	m_do_update = true;
}

glm::vec3 Camera::getForward(float fd)
{
	return m_position + m_forward * fd;
}

void Camera::moveForward()
{
	m_position += m_forward * m_movement_speed;
	m_do_update = true;
}

void Camera::moveBack()
{
	m_position -= m_forward * m_movement_speed;
	m_do_update = true;
}

void Camera::moveLeft()
{
	m_position += m_left * m_movement_speed;	
	m_do_update = true;
}

void Camera::moveRight()
{
	glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));
	right *= m_movement_speed;
	m_position += right;
	
	m_do_update = true;
}

void Camera::moveUp()
{
	NBT_Debug("up: %.02f,%.02f,%.02f", m_up.x, m_up.y, m_up.z);
	m_position += m_up * m_movement_speed;
	m_do_update = true;
}

void Camera::moveDown()
{
	NBT_Debug("up: %.02f,%.02f,%.02f", m_up.x, m_up.y, m_up.z);
	m_position -= m_up * m_movement_speed;
	m_do_update = true;
}


void Camera::update()
{
	//if(!m_do_update)
	//	return;

	m_do_update = false;
	
	//m_mat[0] = glm::vec4(m_left, 0);
	//m_mat[1] = glm::vec4(m_up, 0);
	//m_mat[2] = glm::vec4(m_forward, 0);
	//m_mat[3] = glm::vec4(m_position, 1);
	m_mat = glm::lookAt(m_position, m_position + m_forward, m_up);
}

