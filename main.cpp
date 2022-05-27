// Quick note: GLAD needs to be included first before GLFW.
// Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
using namespace std;

GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath);
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource);

void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

struct Vertex
{
	GLfloat x, y, z;		// Position
	GLubyte r, g, b;		// Color
	GLfloat nx, ny, nz; // Normals
	GLfloat u, v;
};

struct Texture
{
	unsigned int id;
	string type;
};

class Mesh
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setUpMesh();
	}

	void Draw(GLuint shader, glm::mat4 transform)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
private:
	unsigned int VBO, EBO;

	void setUpMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

		glBindVertexArray(0);
	}
};

class Model
{
public:
	Model(string const& path)
	{
		loadModel(path);
	}
	void Draw(GLuint shader, glm::mat4 transform)
	{
		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader, transform);
		}

	}
private:
	vector<Mesh> meshes;
	string directory;

	void loadModel(string path)
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}
	void processNode(aiNode* node, const aiScene* scene)
	{
		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
		//processes vertices
		for(unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			vertex.x = mesh->mVertices[i].x;
			vertex.y = mesh->mVertices[i].y;
			vertex.z = mesh->mVertices[i].z;

			vertex.nx = mesh->mNormals[i].x;
			vertex.ny = mesh->mNormals[i].y;
			vertex.nz = mesh->mNormals[i].z;

			//no textures first
			vertex.u = 0.0f;
			vertex.v = 0.0f;

			vertices.push_back(vertex);
		}
		//process indices
		for(unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for(unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		/*if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			vector<Texture> specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(texture.end(), specularMaps.begin(), specularMaps.end());
		}*/

		return Mesh(vertices, indices, textures);
	}

};


GLuint loadSkybox(std::vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	GLint width, height, numOfChannels;
	for(size_t i = 0; i < faces.size(); ++i)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &numOfChannels, 0);
		if(data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else
			std::cerr << "ERROR loading cubemap texture\n";
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

GLFWwindow* window;
void getInput();

// window size
GLfloat windowWidth, windowHeight;

// camera and movement
GLfloat horizontalAngle = M_PI;
GLfloat verticalAngle = 0.0f;
GLfloat speed = 4.0f;
GLfloat mouseSpeed = 0.8f;
glm::vec3 cameraDirection, cameraRight, cameraUp;

// starting view position
glm::vec3 position(0.0f, 13.0f, 5.0f);
GLdouble xpos, ypos;

// time
GLfloat currentTime, deltaTime, lastTime;

// what to render
int toggle(0);
bool reflectionToggle(false);

int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if(glfwInitStatus == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return 1;
	}

	// Tell GLFW that we prefer to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW that we prefer to use the modern OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Tell GLFW to create a window
	windowWidth = 800;
	windowHeight = 800;
	window = glfwCreateWindow(windowWidth, windowHeight, "FINALS", nullptr, nullptr);
	if(window == nullptr)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Tell GLFW to use the OpenGL context that was assigned to the window that we just created
	glfwMakeContextCurrent(window);

	// Register the callback function that handles when the framebuffer size has changed
	glfwSetFramebufferSizeCallback(window, FramebufferSizeChangedCallback);

	// Tell GLAD to load the OpenGL function pointers
	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// vertex specification
	// Position	 Color  Normal
	Vertex vertices[28];

	// CUBE
	// front
	vertices[0] = { -0.5f, 0.5f, 0.5f, 255, 0, 0 };
	vertices[1] = { 0.5f, 0.5f, 0.5f, 255, 0, 0 };
	vertices[2] = { 0.5f, -0.5f, 0.5f, 255, 0, 0 };
	vertices[3] = { -0.5f, -0.5f, 0.5f, 255, 0, 0 };

	// back
	vertices[4] = { 0.5f, 0.5f, -0.5f, 0, 255, 0 };
	vertices[5] = { -0.5f, 0.5f, -0.5f, 0, 255, 0 };
	vertices[6] = { -0.5f, -0.5f, -0.5f, 0, 255, 0 };
	vertices[7] = { 0.5f, -0.5f, -0.5f, 0, 255, 0 };

	// left
	vertices[8] = { -0.5f, 0.5f, -0.5f, 0, 0, 255 };
	vertices[9] = { -0.5f, 0.5f, 0.5f, 0, 0, 255 };
	vertices[10] = { -0.5f, -0.5f, 0.5f, 0, 0, 255 };
	vertices[11] = { -0.5f, -0.5f, -0.5f, 0, 0, 255 };

	// right
	vertices[12] = { 0.5f, 0.5f, 0.5f, 255, 255, 0 };
	vertices[13] = { 0.5f, 0.5f, -0.5f, 255, 255, 0 };
	vertices[14] = { 0.5f, -0.5f, -0.5f, 255, 255, 0 };
	vertices[15] = { 0.5f, -0.5f, 0.5f, 255, 255, 0 };

	// top
	vertices[16] = { -0.5f, 0.5f, -0.5f, 255, 0, 255 };
	vertices[17] = { 0.5f, 0.5f, -0.5f, 255, 0, 255 };
	vertices[18] = { 0.5f, 0.5f, 0.5f, 255, 0, 255 };
	vertices[19] = { -0.5f, 0.5f, 0.5f, 255, 0, 255 };

	// bottom
	vertices[20] = { -0.5f, -0.5f, 0.5f, 0, 255, 255 };
	vertices[21] = { 0.5f, -0.5f, 0.5f, 0, 255, 255 };
	vertices[22] = { 0.5f, -0.5f, -0.5f, 0, 255, 255 };
	vertices[23] = { -0.5f, -0.5f, -0.5f, 0, 255, 255 };

	// PLANE that faces upwards
	vertices[24] = { -0.5f, 0.5f, -0.5f, 250, 250, 250 };
	vertices[25] = { 0.5f, 0.5f, -0.5f, 250, 250, 250 };
	vertices[26] = { 0.5f, 0.5f, 0.5f, 250, 250, 250 };
	vertices[27] = { -0.5f, 0.5f, 0.5f, 250, 250, 250 };

	// normals and UV
	for(size_t i = 0; i < 28; i += 4)
	{
		GLfloat x0 = vertices[i].x;
		GLfloat y0 = vertices[i].y;
		GLfloat z0 = vertices[i].z;

		GLfloat x1 = vertices[i + 1].x;
		GLfloat y1 = vertices[i + 1].y;
		GLfloat z1 = vertices[i + 1].z;

		GLfloat x2 = vertices[i + 2].x;
		GLfloat y2 = vertices[i + 2].y;
		GLfloat z2 = vertices[i + 2].z;

		GLfloat v0x = x2 - x0;
		GLfloat v0y = y2 - y0;
		GLfloat v0z = z2 - z0;

		GLfloat v1x = x1 - x0;
		GLfloat v1y = y1 - y0;
		GLfloat v1z = z1 - z0;

		glm::vec3 normal;
		normal = glm::cross(glm::vec3(v0x, v0y, v0z), glm::vec3(v1x, v1y, v1z));

		vertices[i].nx = normal.x;
		vertices[i].ny = normal.y;
		vertices[i].nz = normal.z;
		vertices[i + 1].nx = normal.x;
		vertices[i + 1].ny = normal.y;
		vertices[i + 1].nz = normal.z;
		vertices[i + 2].nx = normal.x;
		vertices[i + 2].ny = normal.y;
		vertices[i + 2].nz = normal.z;
		vertices[i + 3].nx = normal.x;
		vertices[i + 3].ny = normal.y;
		vertices[i + 3].nz = normal.z;
	}

	// vertex order for EBO
	GLuint cubeIndices[] = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23 };

	GLuint planeIndices[] = {
			24, 25, 26, 24, 26, 27 };

	// VBO setup
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// EBO setup
	GLuint cubeEbo;
	glGenBuffers(1, &cubeEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	GLuint planeEbo;
	glGenBuffers(1, &planeEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// VAO setup
	GLuint objectVAO;
	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	glBindVertexArray(0);

	// shadow FBO setup
	GLuint shadowFBO;
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	// Depth Texture
	GLuint depthTexture;
	GLuint depthTextureWidth = 1024;
	GLuint depthTextureHeight = 1024;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthTextureWidth, depthTextureHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Framebuffer incomplete...\n";

	std::vector<std::string> faces{
		"./skybox/right.jpg",
		"./skybox/left.jpg",
		"./skybox/top.jpg",
		"./skybox/bottom.jpg",
		"./skybox/front.jpg",
		"./skybox/back.jpg"
	};
	GLuint skybox = loadSkybox(faces);

	GLuint mainShader = CreateShaderProgram("main.vsh", "main.fsh");
	GLuint depthShader = CreateShaderProgram("depth.vsh", "depth.fsh");
	GLuint skyboxShader = CreateShaderProgram("skybox.vsh", "skybox.fsh");

	glUseProgram(mainShader);
	glUniform1i(glGetUniformLocation(mainShader, "skybox"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(glGetUniformLocation(mainShader, "shadowMap"), 1);

	glUseProgram(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	GLint cubeIndicesSize = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
	GLint planeIndicesSize = sizeof(planeIndices) / sizeof(planeIndices[0]);

	lastTime = glfwGetTime();

	// DIRECTIONAL LIGHT
	glm::vec3 directionalLightPosition(0.0f, 6.0f, -4.0f);
	glm::vec3 directionalLightDirection(1.0f, -1.0f, 1.0f);
	glm::vec3 directionalLightAmbient(1.0f, 1.0f, 1.0f);
	glm::vec3 directionalLightDiffuse(0.75f, 0.75f, 0.75f);
	glm::vec3 directionalLightSpecular(0.75f, 0.25f, 0.75f);
	glm::mat4 directionalLightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -50.0f, 50.0f, 0.0f, 30.0f);
	glm::mat4 directionalLightViewMatrix = glm::lookAt(directionalLightPosition, directionalLightPosition + directionalLightDirection, glm::vec3(0, 1, 0));

	Model bedroom = Model("Bedroom.obj");
	Model monkey = Model("Monkey.obj");

	glm::vec3 skyboxColor(0.0f, 0.0f, 0.0f);


	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// identity matrix
	glm::mat4 iMatrix(1.0f);

	// Render loop
	while(!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		getInput();

		if(toggle == 0)
		{
			position.x = glm::sin(currentTime * 0.5f) * 15.0f;
			position.z = glm::cos(currentTime * 0.5f) * 15.0f;
		} else
		{
			position.x = glm::sin(currentTime * 0.5f) * 24.0f;
			position.z = glm::cos(currentTime * 0.5f) * 24.0f;
		}
		cameraDirection = glm::normalize(-position);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		// MVP uniforms
		glm::mat4 viewMatrix = glm::lookAt(position, position + cameraDirection, cameraUp);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), windowWidth / windowHeight, 0.1f, 100.0f);


		// SET OBJECT TRANSFORMS
		glBindVertexArray(objectVAO);
		// cube
		glm::mat4 firstMatrix = glm::scale(iMatrix, glm::vec3(6.0f, 6.0f, 6.0f));
		firstMatrix = glm::translate(firstMatrix, glm::vec3(-1.5f, 0.5f, 0.0f));
		firstMatrix = glm::rotate(firstMatrix, glm::radians(23.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 secondMatrix = glm::scale(iMatrix, glm::vec3(4.5f, 4.5f, 4.5f));
		secondMatrix = glm::translate(secondMatrix, glm::vec3(1.5f, 0.5f, 1.5f));
		secondMatrix = glm::rotate(secondMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 thirdMatrix = glm::scale(iMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
		thirdMatrix = glm::translate(thirdMatrix, glm::vec3(2.5f, 2.0f, -2.0f));
		thirdMatrix = glm::rotate(thirdMatrix, glm::radians(currentTime * 40.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 fourthMatrix = glm::scale(iMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
		fourthMatrix = glm::translate(fourthMatrix, glm::vec3(-2.0f, 3.5f, 5.0f));
		fourthMatrix = glm::rotate(fourthMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		fourthMatrix = glm::rotate(fourthMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 fifthMatrix = glm::scale(iMatrix, glm::vec3(1.5f, 6.0f, 1.5f));
		fifthMatrix = glm::translate(fifthMatrix, glm::vec3(-0.0f, 0.8f, -5.0f));
		fifthMatrix = glm::rotate(fifthMatrix, glm::radians(23.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		fifthMatrix = glm::rotate(fifthMatrix, glm::radians(currentTime * 60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		// plane
		glm::mat4 planeMatrix = glm::scale(iMatrix, glm::vec3(30.0f, 1.0f, 30.0f));
		planeMatrix = glm::translate(planeMatrix, glm::vec3(0, -0.5f, 0));
		// bedroom
		glm::mat4 bedroomMatrix = glm::scale(iMatrix, glm::vec3(7.0f, 7.0f, 7.0f));
		bedroomMatrix = glm::rotate(bedroomMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		// monkey
		glm::mat4 monkeyMatrix = glm::scale(iMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
		// skybox
		glm::mat4 skyboxMatrix = glm::scale(iMatrix, glm::vec3(50.0f, 50.0f, 50.0f));


		// SHADOW PASS
		glUseProgram(depthShader);
		glViewport(0, 0, depthTextureWidth, depthTextureHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(glGetUniformLocation(depthShader, "lightProjection"), 1, GL_FALSE, glm::value_ptr(directionalLightProjectionMatrix));
		glUniformMatrix4fv(glGetUniformLocation(depthShader, "lightView"), 1, GL_FALSE, glm::value_ptr(directionalLightViewMatrix));

		if(toggle == 1)
		{
			bedroom.Draw(depthShader, bedroomMatrix);
		} else if(toggle == 2)
		{
			monkey.Draw(depthShader, monkeyMatrix);
		} else
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(firstMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(secondMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(thirdMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(fourthMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(fifthMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEbo);
			glUniformMatrix4fv(glGetUniformLocation(depthShader, "model"), 1, GL_FALSE, glm::value_ptr(planeMatrix));
			glDrawElements(GL_TRIANGLES, planeIndicesSize, GL_UNSIGNED_INT, 0);
		}

		// RENDER PASS
		glUseProgram(mainShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE1);

		glUniformMatrix4fv(glGetUniformLocation(mainShader, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(glGetUniformLocation(mainShader, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniform3fv(glGetUniformLocation(mainShader, "viewPosition"), 1, glm::value_ptr(position));

		directionalLightDiffuse.x = glm::sin(currentTime * 0.8f) + 1.0f;
		directionalLightDiffuse.y = glm::sin(currentTime * 0.8f) + 1.0f;
		directionalLightDiffuse.z = glm::sin(currentTime * 0.8f) + 1.0f;
		directionalLightAmbient.x = glm::clamp(glm::sin(currentTime * 0.8f) + 1.05f, 0.2f, 0.8f);
		directionalLightAmbient.y = glm::clamp(glm::sin(currentTime * 0.8f) + 1.0f, 0.2f, 0.8f);
		directionalLightAmbient.z = glm::clamp(glm::sin(currentTime * 0.8f) + 1.1f, 0.2f, 0.8f);

		// directional light uniforms
		glUniform3fv(glGetUniformLocation(mainShader, "directionalLightDirection"), 1, glm::value_ptr(directionalLightDirection));
		glUniform3fv(glGetUniformLocation(mainShader, "directionalLightAmbient"), 1, glm::value_ptr(directionalLightAmbient));
		glUniform3fv(glGetUniformLocation(mainShader, "directionalLightDiffuse"), 1, glm::value_ptr(directionalLightDiffuse));
		glUniform3fv(glGetUniformLocation(mainShader, "directionalLightSpecular"), 1, glm::value_ptr(directionalLightSpecular));

		glUniformMatrix4fv(glGetUniformLocation(mainShader, "lightProjection"), 1, GL_FALSE, glm::value_ptr(directionalLightProjectionMatrix));
		glUniformMatrix4fv(glGetUniformLocation(mainShader, "lightView"), 1, GL_FALSE, glm::value_ptr(directionalLightViewMatrix));


		glUniform1i(glGetUniformLocation(mainShader, "reflective"), 0);
		if(reflectionToggle)
			glUniform1i(glGetUniformLocation(mainShader, "reflective"), 1);

		if(toggle == 1)
		{
			bedroom.Draw(mainShader, bedroomMatrix);
		} else if(toggle == 2)
		{
			monkey.Draw(mainShader, monkeyMatrix);
		} else
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(firstMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(secondMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(thirdMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(fourthMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(fifthMatrix));
			glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEbo);
			glUniformMatrix4fv(glGetUniformLocation(mainShader, "model"), 1, GL_FALSE, glm::value_ptr(planeMatrix));
			glDrawElements(GL_TRIANGLES, planeIndicesSize, GL_UNSIGNED_INT, 0);
		}

		// SKYBOX PASS
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyboxShader);
		glBindVertexArray(objectVAO);
		glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxViewMatrix));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniform3fv(glGetUniformLocation(skyboxShader, "skyboxColor"), 1, glm::value_ptr(skyboxColor));
		skyboxColor.x = glm::sin(currentTime * 0.8f) + 1.1f;
		skyboxColor.y = glm::sin(currentTime * 0.8f) + 1.0f;
		skyboxColor.z = glm::sin(currentTime * 0.8f) + 1.25f;

		glActiveTexture(GL_TEXTURE0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "model"), 1, GL_FALSE, glm::value_ptr(skyboxMatrix));
		glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);

		glDepthFunc(GL_LESS);

		// CLEAR
		glBindVertexArray(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteProgram(mainShader);

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &objectVAO);

	glfwTerminate();

	return 0;
}

GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	GLuint vertexShader = CreateShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilePath);
	GLuint fragmentShader = CreateShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	// Check shader program link status
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if(linkStatus != GL_TRUE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "program link error: " << infoLog << std::endl;
	}

	return program;
}

GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
{
	std::ifstream shaderFile(shaderFilePath);
	if(shaderFile.fail())
	{
		std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
		return 0;
	}

	std::string shaderSource;
	std::string temp;
	while(std::getline(shaderFile, temp))
	{
		shaderSource += temp + "\n";
	}
	shaderFile.close();

	return CreateShaderFromSource(shaderType, shaderSource);
}

GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource)
{
	GLuint shader = glCreateShader(shaderType);

	const char* shaderSourceCStr = shaderSource.c_str();
	GLint shaderSourceLen = static_cast<GLint>(shaderSource.length());
	glShaderSource(shader, 1, &shaderSourceCStr, &shaderSourceLen);
	glCompileShader(shader);

	// Check compilation status
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if(compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "shader compilation error: " << infoLog << std::endl;
	}

	return shader;
}

void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}

void getInput()
{
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS and position.y <= 50.0f)
		position.y += 0.1f;
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS and position.y >= -30.0f)
		position.y -= 0.1f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		toggle = (toggle < 2) ? toggle + 1 : 0;
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		reflectionToggle = !reflectionToggle;
}