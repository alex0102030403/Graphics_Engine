#include <fstream>
#include <iostream>
#include <unordered_map>
#include "Model.h"
#include "Input.h"
#include "Utility.h"
#include <array>
#include <string>

Model::Model(const std::string& filename, Grid* parentGrid) : Object(parentGrid)
{
    m_meshes.reserve(20);
    m_buffers.reserve(20);

    std::fstream file(filename, std::ios_base::in);

    if (!file)
    {
        Utility::AddMessage("Error loading model file.");
    }

    Mesh rawMesh;
	std::string line;
    std::string lastMaterialName;
    std::string currentMaterialName; // Track the current material
    std::string lastName;
    std::vector<Face> faces;

    std::vector<std::string> subStrings;
    subStrings.reserve(20);

    while (!file.eof())
    {
        std::getline(file, line);
        subStrings.clear();

        if (!line.empty() && line[0] != '#')
        {
            Utility::ParseString(line, subStrings, ' ');

            // Vertex data
            if (subStrings[0] == "v")
            {
                rawMesh.vertices.push_back(glm::vec3(std::stof(subStrings[1]),
                    std::stof(subStrings[2]),
                    std::stof(subStrings[3])));
                continue;
            }

            // Normal data
            if (subStrings[0] == "vn")
            {
                rawMesh.normals.push_back(glm::vec3(std::stof(subStrings[1]),
                    std::stof(subStrings[2]),
                    std::stof(subStrings[3])));
                continue;
            }

            // Texture coordinate data
            if (subStrings[0] == "vt")
            {
                rawMesh.textureCoords.push_back(glm::vec2(std::stof(subStrings[1]),
                    std::stof(subStrings[2])));
                continue;
            }

            // Faces
            if (subStrings[0] == "f")
            {
                Face face;
                std::vector<std::string> numbers;
                numbers.reserve(10);

                // Parse all vertex groups in the face line
                for (size_t i = 1; i < subStrings.size(); i++)
                {
                    numbers.clear();
                    VertexGroup vertexGroup;
                    Utility::ParseString(subStrings[i], numbers, '/');

                    // Ensure the vertex group has all three components (v/vt/vn)
                    if (numbers.size() == 3)
                    {
                        vertexGroup.v = std::stoi(numbers[0]) - 1;
                        vertexGroup.t = std::stoi(numbers[1]) - 1;
                        vertexGroup.n = std::stoi(numbers[2]) - 1;
                        face.push_back(vertexGroup);
                    }
                }

                // Only add faces with at least 3 vertices
                if (face.size() >= 3)
                {
                    faces.push_back(face);
                }
                continue;
            }

            // Material group
            if (subStrings[0] == "usemtl")
            {
                if (!m_materials.empty())
                {
                    for (const auto& material : m_materials)
                    {
                        if (material.GetName() == subStrings[1])
                        {
                            // If material changes and there are faces, create a new mesh
                            if (!faces.empty() && currentMaterialName != subStrings[1])
                            {
                                Mesh mesh;
                                mesh.name = lastName;
                                mesh.materialName = currentMaterialName;
                                SortVertexData(mesh, rawMesh, faces);
                                m_meshes.push_back(mesh);
                                faces.clear();
                            }
                            currentMaterialName = subStrings[1];
                            lastMaterialName = subStrings[1];
                            break;
                        }
                    }
                }
                continue;
            }

            // Material file
            if (subStrings[0] == "mtllib")
            {
                Material material;
                material.Load(subStrings[1], m_materials);
                continue;
            }

            // Group data
            if (subStrings[0] == "g" || subStrings[0] == "o")
            {
                if (!faces.empty())
                {
                    Mesh mesh;
                    mesh.name = lastName;
                    mesh.materialName = currentMaterialName;
                    SortVertexData(mesh, rawMesh, faces);
                    m_meshes.push_back(mesh);
                    faces.clear();
                }
                lastName = subStrings[1];
                continue;
            }
        }
    }

    file.close();

    if (!faces.empty())
    {
        Mesh mesh;
        mesh.name = lastName;
        mesh.materialName = currentMaterialName;
        SortVertexData(mesh, rawMesh, faces);
        m_meshes.push_back(mesh);
    }

    FillBuffers();
}

void Model::SetColor(const glm::vec4& color)
{
	auto count = 0;
	for (auto& mesh : m_meshes)
	{
		for (auto& col : mesh.colors)
		{
			glm::vec4 newColor(color.r, color.g, color.b, color.a);
			col = newColor;
		}

		m_buffers[count++].FillVBO(Buffer::VBOType::ColorBuffer, &mesh.colors[0].x,
			mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::Ongoing);
	}

	m_color = color;
}

void Model::Render(const Shader& shader)
{
	Object::Render(shader);

	auto count = 0;
	for (auto& buffer : m_buffers)
	{
		buffer.LinkEBO();

		buffer.LinkVBO(shader,
			"vertexIn", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"colorIn", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"textureIn", Buffer::VBOType::TextureBuffer, Buffer::ComponentType::UV, Buffer::DataType::FloatData);
		buffer.LinkVBO(shader,
			"normalIn", Buffer::VBOType::NormalBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);

		for (auto mat : m_materials)
		{
			//Utility::AddMessage("Number of materials: " + std::to_string(m_materials.size()));
			//Utility::AddMessage("Material name: " + mat.GetName());
			if (mat.GetName() == m_meshes[count].materialName)
			{
				
				mat.SendToShader(shader);

				if (mat.IsTextured())
				{
					if (m_isTextured)
					{
						shader.SendData("isTextured", true);
					}

					mat.GetDiffuseMap().Bind();
				}

				else
				{
					shader.SendData("isTextured", false);
				}

				break;
			}
		}

		count++;
		buffer.Render(Buffer::DrawType::Triangles);
	}
}

void Model::FillBuffers()
{
	for (auto& mesh : m_meshes)
	{
		Buffer buffer;
		buffer.CreateBuffer(mesh.indices.size(), true);

		buffer.FillEBO(&mesh.indices[0], mesh.indices.size() * sizeof(GLuint), Buffer::FillType::Once);

		buffer.FillVBO(Buffer::VBOType::VertexBuffer,
			&mesh.vertices[0].x, mesh.vertices.size() * sizeof(glm::vec3), Buffer::FillType::Once);

		buffer.FillVBO(Buffer::VBOType::TextureBuffer,
			&mesh.textureCoords[0].x, mesh.textureCoords.size() * sizeof(glm::vec2), Buffer::FillType::Once);

		buffer.FillVBO(Buffer::VBOType::NormalBuffer,
			&mesh.normals[0].x, mesh.normals.size() * sizeof(glm::vec3), Buffer::FillType::Once);

		for (const auto& vertex : mesh.vertices)
		{
			glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.colors.push_back(color);
		}

		buffer.FillVBO(Buffer::VBOType::ColorBuffer,
			&mesh.colors[0].x, mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::Once);

		m_buffers.push_back(buffer);
	}
}

void Model::SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces)
{
	auto count = 0U;
	std::unordered_map<VertexGroup, GLuint, HashFunction> map;

	for (const auto& face : faces)
	{
		// Skip invalid faces
		if (face.size() < 3) continue;

		// If the face has 3 vertices, process it as a single triangle
		if (face.size() == 3)
		{
			for (size_t i = 0; i < 3; i++)
			{
				auto it = map.find(face[i]);
				if (it == map.end())
				{
					newMesh.vertices.push_back(oldMesh.vertices[face[i].v]);
					newMesh.textureCoords.push_back(oldMesh.textureCoords[face[i].t]);
					newMesh.normals.push_back(oldMesh.normals[face[i].n]);
					newMesh.indices.push_back(count);
					map[face[i]] = count++;
				}
				else
				{
					newMesh.indices.push_back(it->second);
				}
			}
		}
		// If the face has more than 3 vertices, triangulate using a fan approach
		else
		{
			for (size_t i = 1; i < face.size() - 1; i++)
			{
				// Form a triangle with face[0], face[i], face[i+1]
				std::array<VertexGroup, 3> triangle = { face[0], face[i], face[i + 1] };
				for (const auto& vg : triangle)
				{
					auto it = map.find(vg);
					if (it == map.end())
					{
						newMesh.vertices.push_back(oldMesh.vertices[vg.v]);
						newMesh.textureCoords.push_back(oldMesh.textureCoords[vg.t]);
						newMesh.normals.push_back(oldMesh.normals[vg.n]);
						newMesh.indices.push_back(count);
						map[vg] = count++;
					}
					else
					{
						newMesh.indices.push_back(it->second);
					}
				}
			}
		}
	}
}