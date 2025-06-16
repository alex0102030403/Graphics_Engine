#include <gtc\matrix_transform.hpp>
#include "Camera.h"
#include "Input.h"
#include "Shader.h"

Camera::Camera()
{
	m_view = glm::mat4(1.0f);
	m_proj = glm::mat4(1.0f);
	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_transform.SetPosition(0.0f, 3.0f, 20.0f);
	m_yaw = 0.0f;
	m_pitch = 0.0f;
}

void Camera::Set3DView()
{
	auto FOV = 45.0f;
	auto aspectRatio = 1280.0f / 720.0f;
	m_proj = glm::perspective(FOV, aspectRatio, 0.001f, 1000.0f);
}

void Camera::SetSpeed(GLfloat speed)
{
	m_speed = speed;
}

void Camera::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport(x, y, width, height);
}

void Camera::MoveUp()
{
	auto position = m_transform.GetPosition();
	position.y += m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveDown()
{
	auto position = m_transform.GetPosition();
	position.y -= m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveForward(float deltaTime) {
	auto position = m_transform.GetPosition();
	position += m_direction * m_speed * deltaTime; // Use full m_direction, including Y
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveBackward(float deltaTime) {
	auto position = m_transform.GetPosition();
	position -= m_direction * m_speed * deltaTime;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveLeft(float deltaTime) {
	glm::vec3 forward = glm::normalize(glm::vec3(m_direction.x, 0, m_direction.z));
	glm::vec3 left = glm::vec3(forward.z, 0, forward.x);
	auto position = m_transform.GetPosition();
	position += left * m_speed * deltaTime;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveRight(float deltaTime) {
	glm::vec3 forward = glm::normalize(glm::vec3(m_direction.x, 0, m_direction.z));
	glm::vec3 right = glm::vec3(-forward.z, 0, forward.x);
	auto position = m_transform.GetPosition();
	position += right * m_speed * deltaTime;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::LookAt(const glm::vec3& target)
{
	glm::vec3 position = m_transform.GetPosition();
	m_view = glm::lookAt(position, target, m_up);
	m_direction = glm::normalize(target - position);
	m_pitch = asin(m_direction.y);
	m_yaw = atan2(-m_direction.x, -m_direction.z);
}

void Camera::UpdateDirection()
{
	m_direction.x = -sin(m_yaw) * cos(m_pitch);
	m_direction.y = sin(m_pitch);
	m_direction.z = -cos(m_yaw) * cos(m_pitch);
}

void Camera::UpdateRotation(GLfloat deltaYaw, GLfloat deltaPitch)
{
	m_yaw += deltaYaw;
	m_pitch += deltaPitch;

	// Clamp pitch to prevent flipping
	const float maxPitch = glm::pi<float>() / 2.0f - 0.01f;
	if (m_pitch > maxPitch) m_pitch = maxPitch;
	if (m_pitch < -maxPitch) m_pitch = -maxPitch;

	UpdateDirection();
}

void Camera::SendToShader(const Shader& shader)
{
	auto position = m_transform.GetPosition();
	m_view = glm::lookAt(position, position + m_direction, m_up);
	shader.SendData("proj", m_proj);
	shader.SendData("view", m_view);
	shader.SendData("cameraPosition", position.x, position.y, position.z);
}

Utility::Ray Camera::GetPickingRay(float mx, float my, int screenWidth, int screenHeight, int vp_x, int vp_y, int vp_width, int vp_height) {
	glm::vec4 viewport(vp_x, vp_y, vp_width, vp_height);
	// Convert mouse y from SDL (top-left) to OpenGL window coordinates (bottom-left)
	glm::vec3 wincoord(mx, screenHeight - 1 - my, 0.0f); // Near plane
	glm::vec3 nearPoint = glm::unProject(wincoord, GetViewMatrix(), GetProjectionMatrix(), viewport);
	wincoord.z = 1.0f; // Far plane
	glm::vec3 farPoint = glm::unProject(wincoord, GetViewMatrix(), GetProjectionMatrix(), viewport);
	glm::vec3 direction = glm::normalize(farPoint - nearPoint);
	return Utility::Ray{ nearPoint, direction };
}

glm::mat4 Camera::GetViewMatrix() const
{
	return m_view;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return m_proj;
}