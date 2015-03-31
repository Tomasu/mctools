#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "NBT_Debug.h"

Camera::Camera()
{
	NBT_Debug("crap camera ctor");
}

Camera::Camera(const glm::vec3& p, const glm::vec3& f, const glm::vec3& u, float mspeed, float rotspeed) :
	m_position(p), m_forward(glm::normalize(f)), m_up(glm::normalize(u)), m_right(glm::normalize(glm::cross(f, u))), 
	m_world_forward(0.0f, 0.0f, 1.0f), m_world_up(0.0f, 1.0f, 0.0f), m_movement_speed(mspeed), m_rotation_speed(rotspeed), 
	m_do_update(true)
{
	NBT_Debug("main camera ctor");
	al_identity_transform(&m_trans);
}

Camera::~Camera()
{

}

void Camera::look(float xdiff, float ydiff)
{
	NBT_Debug("look: %.02f,%.02f, rs:%.02f", xdiff,ydiff, m_rotation_speed);
	
	glm::vec3 world_up = { 0, 1, 0 };
	
	glm::vec3 tf = glm::normalize(glm::rotate(m_forward, xdiff * m_rotation_speed, world_up ));
	glm::vec3 tu = glm::normalize(glm::rotate(m_up, xdiff * m_rotation_speed, world_up));
	glm::vec3 tr = glm::normalize(glm::cross(tf, tu));
	
	tf = glm::normalize(glm::rotate(tf, ydiff * m_rotation_speed, tr));
	tu = glm::normalize(glm::rotate(tu, ydiff * m_rotation_speed, tr));
	
	glm::vec3 twf = m_forward;
	twf.y = 0;
	twf = glm::normalize(twf);

	float angle = glm::degrees(glm::angle(m_up, m_world_forward));
	NBT_Debug("angle: %.02f", angle);
	
	m_forward = tf;
	m_up = tu;
	m_world_forward = twf;
	
	m_right = glm::normalize(glm::cross(m_world_forward, m_up));

	m_do_update = true;
}

glm::vec3 Camera::getForward(float fd)
{
	return m_position + m_forward * fd;
}

void Camera::moveForward()
{
	m_position += m_world_forward * m_movement_speed;
	m_do_update = true;
}

void Camera::moveBack()
{
	m_position -= m_world_forward * m_movement_speed;
	m_do_update = true;
}

void Camera::moveLeft()
{
	glm::vec3 left = glm::normalize(glm::cross(m_up, m_world_forward));
	m_position += left * m_movement_speed;	
	m_do_update = true;
}

void Camera::moveRight()
{
	m_position += m_right * m_movement_speed;
	m_do_update = true;
}

void Camera::moveUp()
{
	NBT_Debug("up: %.02f,%.02f,%.02f", m_up.x, m_up.y, m_up.z);
	m_position += m_world_up * m_movement_speed;
	m_do_update = true;
}

void Camera::moveDown()
{
	NBT_Debug("up: %.02f,%.02f,%.02f", m_up.x, m_up.y, m_up.z);
	m_position -= m_world_up * m_movement_speed;
	m_do_update = true;
}


void Camera::update()
{
	//if(!m_do_update)
	//	return;

	m_do_update = false;
	
	double x = m_position.x;
	double y = m_position.y;
	double z = m_position.z;
	m_trans.m[0][0] = m_right.x;
	m_trans.m[1][0] = m_right.y;
	m_trans.m[2][0] = m_right.z;
	m_trans.m[3][0] = m_trans.m[0][0] * -x + m_trans.m[1][0] * -y + m_trans.m[2][0] * -z;
	m_trans.m[0][1] = m_up.x;
	m_trans.m[1][1] = m_up.y;
	m_trans.m[2][1] = m_up.z;
	m_trans.m[3][1] = m_trans.m[0][1] * -x + m_trans.m[1][1] * -y + m_trans.m[2][1] * -z;
	m_trans.m[0][2] = -m_forward.x;
	m_trans.m[1][2] = -m_forward.y;
	m_trans.m[2][2] = -m_forward.z;
	m_trans.m[3][2] = m_trans.m[0][2] * -x + m_trans.m[1][2] * -y + m_trans.m[2][2] * -z;
	m_trans.m[0][3] = 0;
	m_trans.m[1][3] = 0;
	m_trans.m[2][3] = 0;
	m_trans.m[3][3] = 1;
	
	//m_mat[0] = glm::vec4(m_left, 0);
	//m_mat[1] = glm::vec4(m_up, 0);
	//m_mat[2] = glm::vec4(m_forward, 0);
	//m_mat[3] = glm::vec4(m_position, 1);
	
	/*
	glm::mat4 mat = glm::lookAt(m_position, m_forward, m_up);
	
	m_trans.m[0][0] = mat[0][0];
	m_trans.m[0][1] = mat[0][1];
	m_trans.m[0][2] = mat[0][2];
	m_trans.m[0][3] = mat[0][3];
	
	m_trans.m[1][0] = mat[1][0];
	m_trans.m[1][1] = mat[1][1];
	m_trans.m[1][2] = mat[1][2];
	m_trans.m[1][3] = mat[1][3];
	
	m_trans.m[2][0] = mat[2][0];
	m_trans.m[2][1] = mat[2][1];
	m_trans.m[2][2] = mat[2][2];
	m_trans.m[2][3] = mat[2][3];
	
	m_trans.m[3][0] = mat[3][0];
	m_trans.m[3][1] = mat[3][1];
	m_trans.m[3][2] = mat[3][2];
	m_trans.m[3][3] = mat[3][3];
	*/
	
	/*
	m_trans.m[0][0] = m_right[0];
	m_trans.m[0][1] = m_right[1];
	m_trans.m[0][2] = m_right[2];
	m_trans.m[0][3] = 0.0f;
	
	m_trans.m[1][0] = m_up[0];
	m_trans.m[1][1] = m_up[1];
	m_trans.m[1][2] = m_up[2];
	m_trans.m[1][3] = 0.0f;
	
	m_trans.m[2][0] = m_forward[0];
	m_trans.m[2][1] = m_forward[1];
	m_trans.m[2][2] = m_forward[2];
	m_trans.m[2][3] = 0.0f;
	
	m_trans.m[3][0] = m_position[0];
	m_trans.m[3][1] = m_position[1];
	m_trans.m[3][2] = m_position[2];
	m_trans.m[3][3] = 1.0f;
	*/
}

