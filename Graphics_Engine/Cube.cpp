#include "Cube.h"
#include "Shader.h"
#include "Input.h"
#include <array> 

Cube::Cube(const std::string& textureFilename, Grid* parentGrid) : Object(parentGrid)
{
	m_transform.SetIdentity();
	GLfloat vertices[] = { -0.5f,  0.5f,  0.5f,
							0.5f,  0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,       //Front face

							0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,       //Back face

						   -0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f, -0.5f,       //Left face

							0.5f,  0.5f,  0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,
							0.5f, -0.5f,  0.5f,       //Right face

						   -0.5f,  0.5f, -0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f,  0.5f,  0.5f,
						   -0.5f,  0.5f,  0.5f,       //Top face

						   -0.5f, -0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
							0.5f, -0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f };    //Bottom face

	GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,      //Front face 
										  
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,      //Back face 
										   
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,      //Left face 
										   
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,      //Right face 
										  
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,      //Top face 
										  
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f };    //Bottom face 

	GLfloat UVs[] = { 0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,      //Front face 

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Back face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Left face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Right face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,	   //Top face

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f };    //Bottom face

	GLfloat normals[] = { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
						  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,      //Front face 

						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,
						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,	    //Back face

						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,	    //Left face

						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,	    //Right face

						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,	    //Top face

						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f };   //Bottom face

	GLuint indices[] = { 0,  1,  3,  3,  1,  2,      //Front face
						 4,  5,  7,  7,  5,  6,      //Back face 
						 8,  9, 11, 11,  9, 10,      //Left face
						12, 13, 15, 15, 13, 14,      //Right face
						16, 17, 19, 19, 17, 18,      //Top face
						20, 21, 23, 23, 21, 22 };    //Bottom face

	m_buffer.CreateBuffer(36, true);
	m_buffer.FillEBO(indices, sizeof(indices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, colors, sizeof(colors), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::TextureBuffer, UVs, sizeof(UVs), Buffer::FillType::Once);
	m_buffer.FillVBO(Buffer::VBOType::NormalBuffer, normals, sizeof(normals), Buffer::FillType::Once);
	m_buffer.LinkEBO();

	m_texture.Load("Textures/" + textureFilename);

	m_material.SetShininess(50.0f);
	m_material.SetAmbient(glm::vec3(0.4f, 0.4f, 0.4f));
	m_material.SetDiffuse(glm::vec3(0.1f, 0.7f, 0.2f));
	m_material.SetSpecular(glm::vec3(0.8f, 0.8f, 0.8f));
}

Cube::~Cube()
{
	m_buffer.DestroyBuffer();
}

void Cube::SetColor(const glm::vec4& color)
{
	std::vector<glm::vec4> colors;

	for (size_t i = 0; i < 24; i++)
	{
		colors.push_back(color);
	}

	m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, 
		colors.data(), 
		colors.size() * sizeof(glm::vec4), 
		Buffer::FillType::Ongoing);

	m_color = color;
}

void Cube::Render(const Shader& shader)
{
	Object::Render(shader);
	shader.SendData("isTextured", m_isTextured);

	m_material.SendToShader(shader);

	m_buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "textureIn", Buffer::VBOType::TextureBuffer, Buffer::ComponentType::UV, Buffer::DataType::FloatData);
	m_buffer.LinkVBO(shader, "normalIn", Buffer::VBOType::NormalBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);

	if (m_isTextured)
	{
		m_texture.Bind();
	}

	m_buffer.Render(Buffer::DrawType::Triangles);
	m_texture.Unbind();
}

bool Cube::IntersectsRay(const Utility::Ray& ray, float& t) {
	const glm::mat4& modelMatrix = m_transform.GetMatrix();

	// Local AABB corners for a unit cube centered at origin
	std::array<glm::vec3, 8> localCorners = {
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f,  0.5f, -0.5f),
		glm::vec3(0.5f,  0.5f, -0.5f),
		glm::vec3(-0.5f, -0.5f,  0.5f),
		glm::vec3(0.5f, -0.5f,  0.5f),
		glm::vec3(-0.5f,  0.5f,  0.5f),
		glm::vec3(0.5f,  0.5f,  0.5f)
	};

	// Transform corners to world space
	std::array<glm::vec3, 8> worldCorners;
	for (int i = 0; i < 8; ++i) {
		worldCorners[i] = glm::vec3(modelMatrix * glm::vec4(localCorners[i], 1.0f));
	}

	// Compute world-space AABB
	glm::vec3 aabbMin = worldCorners[0];
	glm::vec3 aabbMax = worldCorners[0];
	for (int i = 1; i < 8; ++i) {
		aabbMin = glm::min(aabbMin, worldCorners[i]);
		aabbMax = glm::max(aabbMax, worldCorners[i]);
	}

	// Ray-AABB intersection (slab method)
	float tMin = 0.0f;
	float tMax = std::numeric_limits<float>::max();
	for (int i = 0; i < 3; ++i) {
		if (std::abs(ray.direction[i]) < 1e-6f) {
			if (ray.origin[i] < aabbMin[i] || ray.origin[i] > aabbMax[i]) {
				return false;
			}
		}
		else {
			float ood = 1.0f / ray.direction[i];
			float t1 = (aabbMin[i] - ray.origin[i]) * ood;
			float t2 = (aabbMax[i] - ray.origin[i]) * ood;
			if (t1 > t2) std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);
			if (tMin > tMax) return false;
		}
	}

	if (tMin > 0 && tMin < tMax) {
		t = tMin;
		return true;
	}
	return false;
}

void Cube::RenderAABBEdges(const Shader& shader) {
	// Compute AABB as in IntersectsRay
	const glm::mat4& modelMatrix = m_transform.GetMatrix();
	std::array<glm::vec3, 8> localCorners = {
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f,  0.5f, -0.5f),
		glm::vec3(0.5f,  0.5f, -0.5f),
		glm::vec3(-0.5f, -0.5f,  0.5f),
		glm::vec3(0.5f, -0.5f,  0.5f),
		glm::vec3(-0.5f,  0.5f,  0.5f),
		glm::vec3(0.5f,  0.5f,  0.5f)
	};
	std::array<glm::vec3, 8> worldCorners;
	for (int i = 0; i < 8; ++i) {
		worldCorners[i] = glm::vec3(modelMatrix * glm::vec4(localCorners[i], 1.0f));
	}
	glm::vec3 aabbMin = worldCorners[0];
	glm::vec3 aabbMax = worldCorners[0];
	for (int i = 1; i < 8; ++i) {
		aabbMin = glm::min(aabbMin, worldCorners[i]);
		aabbMax = glm::max(aabbMax, worldCorners[i]);
	}
	std::array<glm::vec3, 8> aabbCorners = {
		glm::vec3(aabbMin.x, aabbMin.y, aabbMin.z),
		glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
		glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z),
		glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z),
		glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z),
		glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z),
		glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
		glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z)
	};

	// Define the 12 edges of the AABB
	std::array<std::pair<int, int>, 12> edges = {
		{{0, 1}, {1, 3}, {3, 2}, {2, 0},  // Bottom face
		 {4, 5}, {5, 7}, {7, 6}, {6, 4},  // Top face
		 {0, 4}, {1, 5}, {2, 6}, {3, 7}}  // Vertical edges
	};

	// Prepare vertex data for edges7
    
	std::vector<GLfloat> edgeVertices;
	for (const auto& edge : edges) {
		edgeVertices.push_back(aabbCorners[edge.first].x);
		edgeVertices.push_back(aabbCorners[edge.first].y);
		edgeVertices.push_back(aabbCorners[edge.first].z);
		edgeVertices.push_back(aabbCorners[edge.second].x);
		edgeVertices.push_back(aabbCorners[edge.second].y);
		edgeVertices.push_back(aabbCorners[edge.second].z);
	}

	// Initialize edge buffer once
	if (!m_edgesInitialized) {
		m_edgeBuffer.CreateBuffer(24, false); // 24 vertices for 12 edges (2 vertices per edge)
		m_edgeBuffer.FillVBO(Buffer::VBOType::VertexBuffer, edgeVertices.data(), edgeVertices.size() * sizeof(GLfloat), Buffer::FillType::Once);
		m_edgesInitialized = true;
	}
	else {
		// Update edge buffer if cube transform changes
		m_edgeBuffer.FillVBO(Buffer::VBOType::VertexBuffer, edgeVertices.data(), edgeVertices.size() * sizeof(GLfloat), Buffer::FillType::Ongoing);
	}

	// Set model matrix to identity to prevent double transformation of world-space vertices
	shader.SendData("modelMatrix", glm::mat4(1.0f));

	// Render edges
	shader.SendData("isTextured", false); // No texture for lines
	m_edgeBuffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);

	// Set a distinct color for edges (e.g., red)
	std::vector<glm::vec4> colors;
	const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red color for edges
	for (size_t i = 0; i < 24; i++) {
		colors.push_back(color);
	}
	m_edgeBuffer.FillVBO(Buffer::VBOType::ColorBuffer, colors.data(), colors.size() * sizeof(glm::vec4), Buffer::FillType::Ongoing);
	m_edgeBuffer.LinkVBO(shader, "colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);

	m_edgeBuffer.Render(Buffer::DrawType::Lines);
}