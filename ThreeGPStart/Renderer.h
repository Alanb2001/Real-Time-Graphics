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

	GLuint m_lightProgram{ 0 };

	GLuint m_FXAAProgram{ 0 };

	GLuint m_DOFProgram{ 0 };

	GLuint m_BlurProgram{ 0 };

	GLuint m_ShadowMapProgram{ 0 };

	std::vector<Model> m_Models;

	bool m_wireframe{ false };

	bool m_FXAA{ false };

	bool m_DOF{ false };
	
	GLuint framebufferTexture{0};
	GLuint dofTexture;
	GLuint focusTexture;
	GLuint FBO;
	GLuint RBO;

	GLuint shadowMapFBO;
	GLuint shadowMapWidth = 2048, shadowMapHeight = 2048;
	GLuint shadowMap;

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

