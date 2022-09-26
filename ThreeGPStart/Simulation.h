#pragma once

#include "ExternalLibraryHeaders.h"
#include "Camera.h"

class Renderer;
struct GLFWwindow;

// Simulation class to handle input, updating of the simulation and calling the renderer
class Simulation
{
private:
	// A simple camera
	std::shared_ptr<Helpers::Camera> m_camera;

	// The renderer
	std::shared_ptr<Renderer> m_renderer;

	glm::mat4 mummyTransform = glm::mat4(1);

	glm::mat4 appleTransform = glm::mat4(1);

	glm::mat4 jeepTransform = glm::mat4(1);

	glm::mat4 mummy2Transform = glm::mat4(1);

	glm::mat4 bonesTransform = glm::mat4(1);

	// Remember last update time so we can calculate delta time
	float m_lastTime{ 0 };

	float m_TickTime{ 0.04f };

	// Handle any user input. Return false if program should close.
	bool HandleInput(GLFWwindow* window);
public:
	// Initialise this as well as the renderer, returns false on error
	bool Initialise();

	// Update the simulation (and render) returns false if program should clse
	bool Update(GLFWwindow* window);
};

