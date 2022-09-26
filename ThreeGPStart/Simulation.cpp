#include "Simulation.h"
#include "Camera.h"
#include "Renderer.h"
#include "Model.h"

// Initialise this as well as the renderer, returns false on error
bool Simulation::Initialise()
{
	// Set up camera
	m_camera = std::make_shared<Helpers::Camera>();
	m_camera->Initialise(glm::vec3(0, 200, 900), glm::vec3(0)); // Jeep
	//m_camera->Initialise(glm::vec3(-13.82f, 5.0f, 1.886f), glm::vec3(0.25f, 1.5f, 0), 30.0f,0.8f); // Aqua pig

	// Set up renderer
	m_renderer = std::make_shared<Renderer>();
	return m_renderer->InitialiseGeometry();
}

// Handle any user input. Return false if program should close.
bool Simulation::HandleInput(GLFWwindow* window)
{
	// Not if it is being handled by IMGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (io.WantCaptureKeyboard || io.WantCaptureMouse)
	{
		return true;
	}

	// You can get keypresses like this:
	// if (glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) // means W key pressed
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(-0.02, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(0.02, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(0, 0.02, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(0, -0.02, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(0, 0, 0.02));
	}
	else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		JeepTransform = glm::translate(JeepTransform, glm::vec3(0, 0, -0.02));
	}

	// You can get mouse button input, returned state is GLFW_PRESS or GLFW_RELEASE
	// int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	// Use this to get the mouse position:
	// double xpos, ypos;
	// glfwGetCursorPos(window, &xpos, &ypos);

	// To prevent the mouse leaving the window (and to hide the arrow) you can call:
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// To reenable it use GLFW_CURSOR_NORMAL

	// To see an example of input using GLFW see the camera.cpp file.

	return true;
}

// Update the simulation (and render) returns false if program should close
bool Simulation::Update(GLFWwindow* window)
{
	// Deal with any input
	if (!HandleInput(window))
		return false;

	// Calculate delta time since last called
	// We pass the delta time to the camera and renderer
	float timeNow = (float)glfwGetTime();
	float deltaTime{ timeNow - m_lastTime };
	m_lastTime = timeNow;

	m_TickTime += deltaTime;

	static float angle = 0;

	angle += 0.0005f;
	if (angle > glm::two_pi<float>())
	{
		angle = 0;
	}

	m_renderer->UpdateModelTransform(m_renderer->GetModelIndex("Jeep"), JeepTransform);

	appleTransform = glm::mat4(1);

	// Rotates around x axis		
	appleTransform = glm::translate(appleTransform, glm::vec3{ 200, 400, 0 });
	appleTransform = glm::rotate(appleTransform, angle, glm::vec3{ 0, 1, 0 });

	m_renderer->UpdateModelTransform(m_renderer->GetModelIndex("Apple"), appleTransform);

	mummyTransform = glm::mat4(1);

	// Rotates around x axis		
	mummyTransform = glm::translate(mummyTransform, glm::vec3{ 0, 400, 200 });
	mummyTransform = glm::scale(mummyTransform, glm::vec3{ 30, 30, 30 });

	m_renderer->UpdateModelTransform(m_renderer->GetModelIndex("Mummy"), mummyTransform);

	// The camera needs updating to handle user input internally
	m_camera->Update(window, deltaTime);

	// Render the scene
	m_renderer->Render(*m_camera, deltaTime);

	// IMGUI	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	m_renderer->DefineGUI();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return true;
}
