#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/quaternion.hpp"

#include "NBT_Debug.h"

Camera::Camera()
{
	NBT_Debug("crap camera ctor");
}

Camera::Camera(const glm::vec3& p, const glm::vec3& u, float mspeed, float rotspeed)
{
	NBT_Debug("main camera ctor");
	
	m_position = p;
	m_world_up = u;
	m_look_at = glm::vec3(0, 0, 0);
	m_up = glm::vec3(0, 1, 0);
	m_movement_speed = mspeed;
	m_rotation_speed = rotspeed;
	m_rotation_quat = glm::quat(1, 0, 0, 0);
	m_pitch = 0.0;
	m_yaw = 0.0;
	m_scale = 0.5f;
	m_do_update = true;
}

Camera::~Camera()
{

}

void Camera::look(float xdiff, float ydiff)
{
	NBT_Debug("look: %.02f,%.02f, rs:%.02f", xdiff,ydiff, m_rotation_speed);
	
	float yaw_degrees = m_rotation_speed * ydiff;
	if ((m_pitch > 90 && m_pitch < 270) || (m_pitch < -90 && m_pitch > -270))
		m_yaw -= yaw_degrees;
	else
		m_yaw += yaw_degrees;
		
	float pitch_degrees = m_rotation_speed * xdiff;
	m_yaw += pitch_degrees;
	if(m_yaw > 360.0f)
		m_yaw -= 360.0f;
	else if(m_yaw < -360.0f)
		m_yaw += 360.0f;
	
	m_do_update = true;
}

glm::vec3 Camera::getForward(float fd)
{
	return m_position + m_look_at * fd;
}

void Camera::moveForward()
{
	m_position_delta += m_direction * m_scale;
	m_do_update = true;
}

void Camera::moveBack()
{
	m_position_delta -= m_direction * m_scale;
	m_do_update = true;
}

void Camera::moveLeft()
{
	m_position_delta -= glm::cross(m_direction, m_up) * m_scale;
	m_do_update = true;
}

void Camera::moveRight()
{
	m_position_delta += glm::cross(m_direction, m_up) * m_scale;
	m_do_update = true;
}

void Camera::moveUp()
{
	m_position_delta += m_up * m_scale;
	m_do_update = true;
}

void Camera::moveDown()
{
	m_position_delta -= m_up * m_scale;
	m_do_update = true;
}


void Camera::update()
{
	if(!m_do_update)
		return;

	NBT_Debug("pos: %.02f,%.02f,%.02f", m_position.x, m_position.y, m_position.z);
	NBT_Debug("pitch: %.02f, yaw: %.02f", m_pitch, m_yaw);
	
	m_do_update = false;
	
	m_direction = glm::normalize(m_look_at - m_position);
	
	//detmine axis for pitch rotation
	glm::vec3 axis = glm::cross(m_direction, m_up);

	//compute quaternion for pitch based on the camera pitch angle
	glm::quat pitch_quat = glm::angleAxis(m_pitch, axis);

	//determine heading quaternion from the camera up vector and the heading angle
	glm::quat heading_quat = glm::angleAxis(m_yaw, m_up);

	//add the two quaternions
	glm::quat temp = glm::cross(pitch_quat, heading_quat);
	temp = glm::normalize(temp);

	//update the direction from the quaternion
	m_direction = glm::rotate(temp, m_direction);

	//add the camera delta
	m_position += m_position_delta;
	
	//set the look at to be infront of the camera
	m_look_at = m_position + m_direction * 1.0f;
	
	//damping for smooth camera
	m_yaw *= .5;
	m_pitch *= .5;
	m_position_delta = m_position_delta * .8f;
	
	m_mat = glm::lookAt(m_position, m_look_at, m_up);
}

