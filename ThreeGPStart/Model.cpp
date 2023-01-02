#include "Model.h"
#include "Helper.h"
#include "Mesh.h"
#include "ImageLoader.h"

Model::Model()
{
}

/* This allows you to make a constructor of Model which then allows you to use
	the file and texture path parameters */
Model::Model(std::string filepath, std::string texturepath)
{
	CreateModel(filepath, texturepath);
}

/* This function allows you to call any model and texture using just the file path
	and this makes it so you only need one set of VBO's and VAO's*/
void Model::CreateModel(std::string filepath, std::string texturepath)
{
	Helpers::ModelLoader modelLoader;
	if (!modelLoader.LoadFromFile(filepath))
	{
		return;
	}

	for (const Helpers::Mesh& mesh : modelLoader.GetMeshVector())
	{
		Mesh smesh;
		smesh.numElements = (GLuint)mesh.elements.size();

		GLuint verticesVBO;
		glGenBuffers(1, &verticesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

		GLuint normalsVBO;
		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);

		GLuint texturesVBO;
		glGenBuffers(1, &texturesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texturesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);

		GLuint elementsVBO;
		glGenBuffers(1, &elementsVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &smesh.VAO);
		glBindVertexArray(smesh.VAO);

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

		glBindBuffer(GL_ARRAY_BUFFER, texturesVBO);
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
		if (image.Load(texturepath))
		{
			glGenTextures(1, &smesh.tex);
			glBindTexture(GL_TEXTURE_2D, smesh.tex);
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

		m_Meshs.push_back(smesh);

		Helpers::CheckForGLError();
	}
}

void Model::UpdateModel(glm::mat4 modelTransforms)
{
	m_ModelTransform = modelTransforms;
}

glm::mat4 Model::GetModelTransform()
{
	return m_ModelTransform;
}