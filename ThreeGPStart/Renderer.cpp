#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"
#include "Model.h"

Renderer::Renderer()
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	for (Model& mod : m_Models)
	{
		for (Mesh& mesh : mod.m_Meshs)
		{
			glDeleteBuffers(1, &mesh.VAO);
		}
	}
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		ImGui::Begin("Real-Time Graphics");                    // Create a window called "3GP" and append into it.

		ImGui::Text("Visibility.");             // Display some text (you can use a format strings too)	

		ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}

	Helpers::CheckForGLError();
}

void Renderer::CreateTerrain(int size)
{
	Model terrain;

	unsigned int numCellsX = 32;
	unsigned int numCellsZ = 32;

	int numVertsX = numCellsX + 1;
	int numVertsZ = numCellsZ + 1;

	int numTrisX = numCellsX * 2;
	int numTrisZ = numCellsZ * 2;

	int cellSizeX = size / numCellsX;
	int cellSizeZ = size / numCellsZ;

	glm::vec3 starPos{ -size / 2.0f, 0, size / 2.0f };

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texcoord;

	float tiles = 10.0f;

	Helpers::ImageLoader heightmap;
	if (!heightmap.Load("Data\\Heightmaps\\curvy.gif"))
	{
		return;
	}

	unsigned char* texels = (unsigned char*)heightmap.GetData();

	for (int z = 0; z < numVertsZ; z++)
	{
		for (int x = 0; x < numVertsX; x++)
		{
			glm::vec3 pos{ starPos };

			pos.x += cellSizeX * x;
			pos.z -= cellSizeZ * z;

			float u = (float)x / (numVertsX - 1);
			float v = (float)z / (numVertsZ - 1);

			int heightMapX = (int)(u * (heightmap.Width() - 1));
			int heightMapY = (int)(v * (heightmap.Height() - 1));

			int offset = (heightMapX + heightMapY * heightmap.Width()) * 4;
			pos.y = texels[offset];

			vertices.push_back(pos);

			u *= tiles;
			v *= tiles;

			texcoord.push_back(glm::vec2(u, v));
		}
	}

	std::vector<GLuint> elements;

	bool toggle = true;

	for (GLuint z = 0; z < numCellsZ; z++)
	{
		for (GLuint x = 0; x < numCellsX; x++)
		{
			GLuint startVertex = z * numVertsX + x;

			if (toggle)
			{
				elements.push_back(startVertex);
				elements.push_back(startVertex + 1);
				elements.push_back(startVertex + 1 + numVertsX);

				elements.push_back(startVertex);
				elements.push_back(startVertex + 1 + numVertsX);
				elements.push_back(startVertex + numVertsX);
			}
			else
			{
				elements.push_back(startVertex);
				elements.push_back(startVertex + 1);
				elements.push_back(startVertex + numVertsX);

				elements.push_back(startVertex + 1);
				elements.push_back(startVertex + 1 + numVertsX);
				elements.push_back(startVertex + numVertsX);
			}
			toggle = !toggle;
		}
		toggle = !toggle;
	}

	std::vector<glm::vec3> normals(vertices.size());
	std::fill(normals.begin(), normals.end(), glm::vec3(0, 0, 0));

	for (size_t index = 0; index < elements.size(); index += 3)
	{
		int index1 = elements[index + 0];
		int index2 = elements[index + 1];
		int index3 = elements[index + 2];

		glm::vec3 v0 = vertices[index1];
		glm::vec3 v1 = vertices[index2];
		glm::vec3 v2 = vertices[index3];

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;

		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		normals[index1] += normal;
		normals[index2] += normal;
		normals[index3] += normal;
	}

	for (glm::vec3& n : normals)
	{
		n = glm::normalize(n);
	}

	Mesh mesh;

	GLuint verticesVBO;
	glGenBuffers(1, &verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	GLuint normalsVBO;
	glGenBuffers(1, &normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);

	GLuint coordsVBO;
	glGenBuffers(1, &coordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, coordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texcoord.size(), texcoord.data(), GL_STATIC_DRAW);

	GLuint elementsVBO;
	glGenBuffers(1, &elementsVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elements.size(), elements.data(), GL_STATIC_DRAW);

	mesh.numElements = (GLuint)elements.size();

	glGenVertexArrays(1, &mesh.VAO);
	glBindVertexArray(mesh.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ARRAY_BUFFER, coordsVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);

	glBindVertexArray(0);

	Helpers::ImageLoader image;
	if (image.Load("Data\\Textures\\grass11.bmp"))
	{
		glGenTextures(1, &mesh.tex);
		glBindTexture(GL_TEXTURE_2D, mesh.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Texture load error" << std::endl;
	}

	terrain.m_ModelName = "Terrain";

	terrain.m_Meshs.push_back(mesh);

	m_Models.push_back(terrain);

	Helpers::CheckForGLError();
}

// Loads, compiles and links the shaders and creates a program object to host them
bool Renderer::CreateProgram()
{
	// Creates a new program (returns a unqiue id)
	m_program = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/vertex_shader.glsl") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/fragment_shader.glsl") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_program, vertex_shader);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	return !Helpers::CheckForGLError();
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram())
	{
		return false;
	}

	CreateTerrain(3000);

	Model jeep("Data\\Models\\Jeep\\jeep.obj", "Data\\Models\\Jeep\\jeep_army.jpg");
	Model mummy("Data\\Models\\Mummy\\mummy.x", "Data\\Models\\Mummy\\mummy.bmp");
	Model apple("Data\\Models\\Apple\\apple.obj", "Data\\Models\\Apple\\2.jpg");
	Model mummy2("Data\\Models\\Mummy\\mummy.x", "Data\\Models\\Spider\\circut color.bmp");
	Model bones("Data\\Models\\Bones\\bones_idle.x", "Data\\Models\\Bones\\bones.BMP");

	jeep.m_ModelName = "Jeep";
	mummy.m_ModelName = "Mummy";
	apple.m_ModelName = "Apple";
	mummy2.m_ModelName = "Mummy2";
	bones.m_ModelName = "Bones";

	m_Models.push_back(jeep);
	m_Models.push_back(mummy);
	m_Models.push_back(apple);
	m_Models.push_back(mummy2);
	m_Models.push_back(bones);

	Helpers::CheckForGLError();

	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime)
{
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 6000.0f);

	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;

	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");

	glUseProgram(m_program);

	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	GLuint camera_position_id = glGetUniformLocation(m_program, "camera_position");
	glm::vec3 camera_position = camera.GetPosition();
	glUniform3fv(camera_position_id, 1, glm::value_ptr(camera_position));

	for (Model& mod : m_Models)
	{
		glm::mat4 model_xform = glm::mat4(1);
		model_xform *= mod.GetModelTransform();
		GLuint model_xform_id = glGetUniformLocation(m_program, "model_xform");
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));
	
		for (Mesh& mesh : mod.m_Meshs)
		{
			if (mesh.tex)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.tex);
				glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}
	
			glBindVertexArray(mesh.VAO);
			glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, (void*)0);
		}
	}

	// Always a good idea, when debugging at least, to check for GL errors each frame
	Helpers::CheckForGLError();
}

void Renderer::UpdateModelTransform(int index, glm::mat4 transform)
{
	m_Models[index].UpdateModel(transform);
}

int Renderer::GetModelIndex(std::string modelName)
{
	for (int i = 0; i < m_Models.size(); i++)
	{
		if (m_Models[i].m_ModelName == modelName)
		{
			return i;
		}
	}
	return -1;
}

