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
	glDeleteProgram(m_lightProgram);
	glDeleteProgram(m_FXAAProgram);
	glDeleteProgram(m_DOFProgram);
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

		ImGui::Checkbox("FXAA",	&m_FXAA);

		ImGui::Checkbox("DOF", &m_DOF);

		ImGui::InputScalar("Focus", ImGuiDataType_Float, &focus, &focusStep, &focusStep, "%.6f");

		ImGui::InputScalar("Aperture", ImGuiDataType_Float, &aperture, &apertureStep, &apertureStep, "%.6f");
		
		ImGui::InputScalar("FocalLength", ImGuiDataType_Float, &focalLength, &focalLengthStep, &focalLengthStep, "%.6f");
	
		ImGui::InputScalar("Iterations", ImGuiDataType_S32, &iterations, &iterationsStep, &iterationsStep, "%d");
		
		ImGui::InputScalar("ApertureBlades", ImGuiDataType_S32, &apertureBlades, &apertureBladesStep, &apertureBladesStep, "%d");

		ImGui::InputScalar("BokehSqueeze", ImGuiDataType_Float, &bokehSqueeze, &bokehSqueezeStep, &bokehSqueezeStep, "%.6f");
		
		ImGui::InputScalar("BokehSqueezeFalloff", ImGuiDataType_Float, &bokehSqueezeFalloff, &bokehSqueezeFalloffStep, &bokehSqueezeFalloffStep, "%.6f");

		ImGui::InputScalar("AspectRatio", ImGuiDataType_Float, &aspectRatio, &aspectRatioStep, &aspectRatioStep, "%.6f");

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

void Renderer::CreateFrameBuffer()
{
	glUseProgram(m_FXAAProgram);
	glUniform1i(glGetUniformLocation(m_FXAAProgram, "sampler_tex"), 0);

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer error: " << fboStatus << std::endl;
	}
}

void Renderer::CreateFrameBufferMultipleTextures()
{
	glUseProgram(m_DOFProgram);
	glUniform1i(glGetUniformLocation(m_DOFProgram, "shadedPass"), 0);
	glUniform1i(glGetUniformLocation(m_DOFProgram, "linearDistance"), 0);

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	glGenTextures(1, &dofTexture);
	glBindTexture(GL_TEXTURE_2D, dofTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, dofTexture, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer error: " << fboStatus << std::endl;
	}

	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);

		fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "PingPong Framebuffer error: " << fboStatus << std::endl;
		}
	}
}

GLuint shadowMapFBO;
GLuint shadowMapWidth = 2048, shadowMapHeight = 2048;
GLuint shadowMap;

void Renderer::CreateShadowMapFrameBuffer()
{
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	for (GLuint i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	//GLfloat clampColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColour);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Shadow Map Framebuffer error: " << fboStatus << std::endl;
	}
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

	// Attach the fragment shader (copies it)
	glAttachShader(m_program, fragment_shader);
	//glBindAttribLocation(m_program, 0, "fragment_colour");

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_program))
		return false;

	// Creates a new program (returns a unqiue id)
	m_lightProgram = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint lights_VS{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/lights_VS.glsl") };
	GLuint lights_FS{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/lights_FS.glsl") };
	if (lights_VS == 0 || lights_FS == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_lightProgram, lights_VS);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself

	// Attach the fragment shader (copies it)
	glAttachShader(m_lightProgram, lights_FS);

	// Done with the originals of these as we have made copies
	glDeleteShader(lights_VS);
	glDeleteShader(lights_FS);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_lightProgram))
		return false;

	// Creates a new program (returns a unqiue id)
	m_FXAAProgram = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint FXAA_VS{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/FXAA_VS.glsl") };
	GLuint FXAA_FS{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/FXAA_FS.glsl") };
	if (FXAA_VS == 0 || FXAA_FS == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_FXAAProgram, FXAA_VS);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself

	// Attach the fragment shader (copies it)
	glAttachShader(m_FXAAProgram, FXAA_FS);

	// Done with the originals of these as we have made copies
	glDeleteShader(FXAA_VS);
	glDeleteShader(FXAA_FS);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_FXAAProgram))
		return false;

	m_DOFProgram = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint DOF_VS{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/DOF_VS.glsl") };
	GLuint DOF_FS{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/DOF_FS.glsl") };
	if (DOF_VS == 0 || DOF_FS == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_DOFProgram, DOF_VS);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself

	// Attach the fragment shader (copies it)
	glAttachShader(m_DOFProgram, DOF_FS);

	// Done with the originals of these as we have made copies
	glDeleteShader(DOF_VS);
	glDeleteShader(DOF_FS);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_DOFProgram))
		return false;

	m_BlurProgram = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint Blur_VS{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/Blur_VS.glsl") };
	GLuint Blur_FS{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/Blur_FS.glsl") };
	if (Blur_VS == 0 || Blur_FS == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_BlurProgram, Blur_VS);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself

	// Attach the fragment shader (copies it)
	glAttachShader(m_BlurProgram, Blur_FS);

	// Done with the originals of these as we have made copies
	glDeleteShader(Blur_VS);
	glDeleteShader(Blur_FS);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_BlurProgram))
		return false;

	m_ShadowMapProgram = glCreateProgram();

	// Loads and creates vertex and fragment shaders
	GLuint ShadowMap_VS{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/ShadowMap_VS.glsl") };
	GLuint ShadowMap_FS{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/ShadowMap_FS.glsl") };
	GLuint ShadowMap_GEOM{ Helpers::LoadAndCompileShader(GL_GEOMETRY_SHADER, "Data/Shaders/ShadowMap_GEOM.glsl") };
	if (ShadowMap_VS == 0 || ShadowMap_FS == 0 || ShadowMap_GEOM == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(m_ShadowMapProgram, ShadowMap_VS);

	// The attibute 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself

	// Attach the fragment shader (copies it)
	glAttachShader(m_ShadowMapProgram, ShadowMap_FS);
	glAttachShader(m_ShadowMapProgram, ShadowMap_GEOM);

	// Done with the originals of these as we have made copies
	glDeleteShader(ShadowMap_VS);
	glDeleteShader(ShadowMap_FS);
	glDeleteShader(ShadowMap_GEOM);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(m_ShadowMapProgram))
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
	
	CreateFrameBuffer();

	CreateFrameBufferMultipleTextures();

	CreateShadowMapFrameBuffer();

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
	// Bind the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	// Specify the color of the background
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	// Clean the back buffer and depth buffer
	glClearDepth(1.0f);     
	// Enable depth testing since it's disabled when drawing the framebuffer rectangle
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Clear buffers from previous frame;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 6000.0f);
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 combined_xform = projection_xform * view_xform;
	GLuint combined_xform_id = glGetUniformLocation(m_program, "combined_xform");

	glUseProgram(m_program);

	glm::vec3 camera_position = camera.GetPosition();
	GLuint camera_position_id = glGetUniformLocation(m_program, "camera_position");
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
	
	glUseProgram(m_lightProgram);
	
	combined_xform = projection_xform * view_xform;
	combined_xform_id = glGetUniformLocation(m_lightProgram, "combined_xform");

	glm::vec3 lightPosition = glm::vec3(0, 50, 0);
	GLuint lightPositionID = glGetUniformLocation(m_lightProgram, "light[0].light_position");
	glUniform3fv(lightPositionID, 1, glm::value_ptr(lightPosition));

	glm::vec3 lightColour = glm::vec3(1, 0, 0);
	GLuint lightColourID = glGetUniformLocation(m_lightProgram, "light[0].light_colour");
	glUniform3fv(lightColourID, 1, glm::value_ptr(lightColour));

	lightPosition = glm::vec3(500, 500, 0);
	lightPositionID = glGetUniformLocation(m_lightProgram, "light[1].light_position");
	glUniform3fv(lightPositionID, 1, glm::value_ptr(lightPosition));

	lightColour = glm::vec3(1, 1, 1);
	lightColourID = glGetUniformLocation(m_lightProgram, "light[1].light_colour");
	glUniform3fv(lightColourID, 1, glm::value_ptr(lightColour));

	lightPosition = glm::vec3(0, 500, 500);
	lightPositionID = glGetUniformLocation(m_lightProgram, "light[2].light_position");
	glUniform3fv(lightPositionID, 1, glm::value_ptr(lightPosition));

	lightColour = glm::vec3(0, 0, 1);
	lightColourID = glGetUniformLocation(m_lightProgram, "light[2].light_colour");
	glUniform3fv(lightColourID, 1, glm::value_ptr(lightColour));

	camera_position_id = glGetUniformLocation(m_lightProgram, "camera_position");
	camera_position = camera.GetPosition();
	glUniform3fv(camera_position_id, 1, glm::value_ptr(camera_position));

	glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	for (Model& mod : m_Models)
	{
		glm::mat4 model_xform = glm::mat4(1);
		model_xform *= mod.GetModelTransform();
		GLuint model_xform_id = glGetUniformLocation(m_lightProgram, "model_xform");
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));

		for (Mesh& mesh : mod.m_Meshs)
		{
			if (mesh.tex)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh.tex);
				glUniform1i(glGetUniformLocation(m_lightProgram, "sampler_tex"), 0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			glBindVertexArray(mesh.VAO);
			glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, (void*)0);
		}
	};

	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Draw the framebuffer rectangle
	glUseProgram(m_FXAAProgram);
	glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
	glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	if (m_FXAA)
	{
		GLuint fxaaOn = 1;
		GLuint fxaaOnID = glGetUniformLocation(m_FXAAProgram, "u_fxaaOn");
		glUniform1i(fxaaOnID, fxaaOn);
	}
	else
	{
		GLuint fxaaOn = 0;
		GLuint fxaaOnID = glGetUniformLocation(m_FXAAProgram, "u_fxaaOn");
		glUniform1i(fxaaOnID, fxaaOn);
	}

	glm::vec2 texelStep = glm::vec2(1.0f / 1280, 1.0f / 720);
	GLuint texelStepID = glGetUniformLocation(m_FXAAProgram, "u_texelStep");
	glUniform2fv(texelStepID, 1, glm::value_ptr(texelStep));

	GLuint showEdges = 0;
	GLuint showEdgesID = glGetUniformLocation(m_FXAAProgram, "u_showEdges");
	glUniform1i(showEdgesID, showEdges);

	GLfloat lumaThreshold = 0.5f;
	GLuint lumaThresholdID = glGetUniformLocation(m_FXAAProgram, "u_lumaThreshold");
	glUniform1f(lumaThresholdID, lumaThreshold);

	GLfloat mulReduce = 1.0f / 8.0f;
	GLuint mulReduceID = glGetUniformLocation(m_FXAAProgram, "u_mulReduce");
	glUniform1f(mulReduceID, mulReduce);

	GLfloat minReduce = 1.0f / 128.0f;
	GLuint minReduceID = glGetUniformLocation(m_FXAAProgram, "u_minReduce");
	glUniform1f(minReduceID, minReduce);

	GLfloat maxSpan = 8.0f;
	GLuint maxSpanID = glGetUniformLocation(m_FXAAProgram, "u_maxSpan");
	glUniform1f(maxSpanID, maxSpan);

	if (m_DOF)
	{
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glUseProgram(m_DOFProgram);
		//bool blur = true, firstIteration = true;
		//for (GLuint i = 0; i < 2; i++)
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[blur]);

		//	if (firstIteration)
		//	{
		//		glBindTexture(GL_TEXTURE_2D, dofTexture);
		//		firstIteration = false;
		//	}
		//	else
		//	{
		//		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!blur]);
		//	}

		//	glDisable(GL_DEPTH_TEST);
		//	glDisable(GL_CULL_FACE); // prevents framebuffer rectangle from being discarded
		//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//	blur = !blur;
		//}

		// Bind the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Draw the framebuffer rectangle
		glUseProgram(m_DOFProgram);
		glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
		glDisable(GL_CULL_FACE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, dofTexture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glm::vec2 pixelSize = glm::vec2(1.0f / 1280, 1.0f / 720);
		GLuint pixelSizeID = glGetUniformLocation(m_DOFProgram, "pixelSize");
		glUniform2fv(pixelSizeID, 1, glm::value_ptr(pixelSize));

		focus;
		GLuint focusID = glGetUniformLocation(m_DOFProgram, "focus");
		glUniform1f(focusID, focus);

		aperture;
		GLuint apertureID = glGetUniformLocation(m_DOFProgram, "aperture");
		glUniform1f(apertureID, aperture);

		focalLength;
		GLuint focalLengthID = glGetUniformLocation(m_DOFProgram, "focalLength");
		glUniform1f(focalLengthID, focalLength);

		iterations;
		GLuint iterationsID = glGetUniformLocation(m_DOFProgram, "iterations");
		glUniform1i(iterationsID, iterations);

		apertureBlades;
		GLuint apertureBladesID = glGetUniformLocation(m_DOFProgram, "apertureBlades");
		glUniform1i(apertureBladesID, apertureBlades);

		bokehSqueeze;
		GLuint bokehSqueezeID = glGetUniformLocation(m_DOFProgram, "bokehSqueeze");
		glUniform1f(bokehSqueezeID, bokehSqueeze);

		bokehSqueezeFalloff;
		GLuint bokehSqueezeFalloffID = glGetUniformLocation(m_DOFProgram, "bokehSqueezeFalloff");
		glUniform1f(bokehSqueezeFalloffID, bokehSqueezeFalloff);

		aspectRatio;
		GLuint aspectRatioID = glGetUniformLocation(m_DOFProgram, "aspectRatio");
		glUniform1f(aspectRatioID, aspectRatio);
	}
	
	//glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	//glm::mat4 shadowTransforms[] =
	//{
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	//	shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	//};

	//glUseProgram(m_ShadowMapProgram);
	//glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[0]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[0]));
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[1]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[1]));
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[2]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[2]));
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[3]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[3]));
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[4]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[4]));
	//glUniformMatrix4fv(glGetUniformLocation(m_ShadowMapProgram, "shadowTransforms[5]"), 1, GL_FALSE, glm::value_ptr(shadowTransforms[5]));
	//glUniform3fv(glGetUniformLocation(m_ShadowMapProgram, "lightPos"), 1, glm::value_ptr(lightPosition));
	//glUniform1f(glGetUniformLocation(m_ShadowMapProgram, "far_plane"), 100.0f);

	//glActiveTexture(GL_TEXTURE + 2);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
	//glUniform1i(glGetUniformLocation(m_ShadowMapProgram, "shadowMap"), 2);

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