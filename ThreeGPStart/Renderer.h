#pragma once

#include "ExternalLibraryHeaders.h"
#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"

class Model;

struct PerFrameUniforms
{
	glm::vec3 light_position;
	float light_range;
	glm::vec3 light_direction;
	glm::vec3 cam_pos;
	glm::vec3 ambeint_light;
};

struct PerModelUniforms
{
	glm::mat4 projection_xform;
	glm::mat4 model_xform;
	glm::mat4 view_xform;
	glm::vec3 material_colour;
	float pad1_;
};

class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_program{ 0 };

	GLuint m_lightProgram{ 0 };

	std::vector<Model> m_Models;

	GLuint per_frame_ubo_{ 0 };
	GLuint per_model_ubo_{ 0 };

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

