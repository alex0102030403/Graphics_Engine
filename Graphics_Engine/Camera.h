#pragma once

#include "gl.h"
#include <glm.hpp>
#include "Shader.h"
#include "Transform.h"
#include "Utility.h"

class Camera
{

public:

	Camera();

	void Set3DView();
	void SetSpeed(GLfloat speed);
	void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);

	void MoveUp();
	void MoveDown();
	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void LookAt(const glm::vec3& target);

	Transform& GetTransform() { return m_transform; }
	
	void Update() {}
	void SendToShader(const Shader& shader);

	void UpdateRotation(GLfloat deltaYaw, GLfloat deltaPitch);
	void UpdateDirection();

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	Utility::Ray GetPickingRay(float mx, float my, int screenWidth, int screenHeight, int vp_x, int vp_y, int vp_width, int vp_height);

protected:

	Transform m_transform;
	glm::mat4 m_view;
	glm::mat4 m_proj;
	glm::vec3 m_up;
	glm::vec3 m_direction;
	GLfloat m_speed = 0.0f;

	GLfloat m_yaw = 0.0f;
	GLfloat m_pitch = 0.0f;

};