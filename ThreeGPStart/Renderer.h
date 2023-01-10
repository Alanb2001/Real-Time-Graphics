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

	GLuint m_FXAAProgram{ 0 };

	GLuint m_DOFProgram{ 0 };

	GLuint m_BlurProgram{ 0 };

	GLuint m_ShadowMapProgram{ 0 };

	std::vector<Model> m_Models;

	GLuint per_frame_ubo_{ 0 };
	GLuint per_model_ubo_{ 0 };
	
	bool m_wireframe{ false };

	bool m_FXAA{ false };

	bool m_DOF{ false };
	
	GLuint framebufferTexture;
	GLuint dofTexture;
	GLuint focusTexture;
	GLuint FBO;
	GLuint rectVAO, rectVBO;
	GLuint RBO;
	GLuint pingpongFBO[2];
	GLuint pingpongBuffer[2];

	GLfloat focus = 14.0f;
	GLfloat focusStep = 0.1f;
	GLfloat aperture = 1.4f;
	GLfloat apertureStep = 0.1f;
	GLfloat focalLength = 0.050f;
	GLfloat focalLengthStep = 0.001f;
	GLuint iterations = 64;
	GLuint iterationsStep = 1;
	GLuint apertureBlades = 5;
	GLuint apertureBladesStep = 1;
	GLfloat bokehSqueeze = 0.0f;
	GLfloat bokehSqueezeStep = 0.1f;
	GLfloat bokehSqueezeFalloff = 1.0f;
	GLfloat bokehSqueezeFalloffStep = 0.1f;
	GLfloat aspectRatio = 1.777f;
	GLfloat aspectRatioStep = 0.001f;

	bool CreateProgram();
public:
	Renderer();
	~Renderer();
	void DefineGUI();

	void CreateTerrain(int size);
	void CreateFrameBuffer();
	void CreateFrameBufferMultipleTextures();
	void CreateShadowMapFrameBuffer();

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime);

	void UpdateModelTransform(int index, glm::mat4 transform);

	int GetModelIndex(std::string modelName);
};

