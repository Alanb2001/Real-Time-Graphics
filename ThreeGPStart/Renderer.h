#pragma once

#include "ExternalLibraryHeaders.h"
#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

class Model;

class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_program{ 0 };

	std::vector<Model> m_Models;

	bool m_wireframe{ false };

	bool CreateProgram();
public:
	Renderer();
	~Renderer();
	void DefineGUI();

	void CreateTerrain(int size);

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);

	void UpdateModelTransform(int index, glm::mat4 transform);

	int GetModelIndex(std::string modelName);
};

