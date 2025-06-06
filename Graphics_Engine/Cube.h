#pragma once

#include <string>
#include "Buffer.h"
#include "Material.h"
#include "Object.h"
#include "Texture.h"

class Cube : public Object
{

public:

	Cube(const std::string& textureFilename, Grid* parentGrid = nullptr);
	~Cube() override;

	void SetColor(const glm::vec4& color) override;

	void Update() override {}
	void Render(const Shader& shader) override;
	void RenderAABBEdges(const Shader& shader) override; // New method to render AABB edges

	bool IntersectsRay(const Utility::Ray& ray, float& t) override;
	glm::vec3 GetBoundingSphereCenter() const override { return m_transform.GetPosition(); }
	float GetBoundingSphereRadius() const override { return 0.5f; } // Assuming a unit cube with radius 0.5

private:

	Buffer m_buffer;
	Texture m_texture;
	Material m_material;

	// For AABB edge rendering
	Buffer m_edgeBuffer;
	bool m_edgesInitialized = false;

};