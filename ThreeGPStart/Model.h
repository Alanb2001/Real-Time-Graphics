#pragma once
#include "ExternalLibraryHeaders.h"

struct Mesh
{
	GLuint VAO{ 0 };
	GLuint numElements{ 0 };
	GLuint tex{ 0 };
};

class Model
{
private:
	glm::mat4 m_ModelTransform = glm::mat4(1);
public:
	std::vector<Mesh> m_Meshs;
	std::string m_ModelName{ "" };

	Model();
	Model(std::string filepath, std::string texturepath);

	void CreateModel(std::string filepath, std::string texturepath);
	void UpdateModel(glm::mat4 modelTransforms);

	glm::mat4 GetModelTransform();
};

