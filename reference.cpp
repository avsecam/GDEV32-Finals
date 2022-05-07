// Quick note: GLAD needs to be included first before GLFW.
// Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ---------------
// Function declarations
// ---------------

/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource);

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

/// <summary>
/// Struct containing data about a vertex
/// </summary>
struct Vertex
{
	GLfloat x, y, z;	  // Position
	GLubyte r, g, b;		// Color
	GLfloat u, v;		  	// UV coordinates
	GLfloat nx, ny, nz;	// Normals
};

/// <summary>
/// Main function.
/// </summary>
/// <returns>An integer indicating whether the program ended successfully or not.
/// A value of 0 indicates the program ended succesfully, while a non-zero value indicates
/// something wrong happened during execution.</returns>
int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if (glfwInitStatus == GLFW_FALSE)
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

	// Tell GLFW to create a window
	float windowWidth = 800;
	float windowHeight = 800;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Many Lights 💡", nullptr, nullptr);
	if (window == nullptr)
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
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// vertex specification
	// 							Position									Color								 UV

	// cube
	Vertex cubeVertices[24];

	// front
	cubeVertices[0] = { -0.2f, 0.2f, 0.2f,        255, 255, 0,         0.0f, 1.0f };
	cubeVertices[1] = { 0.2f, 0.2f, 0.2f,         255, 255, 0,         1.0f, 1.0f };
	cubeVertices[2] = { 0.2f, -0.2f, 0.2f,        255, 255, 0,         1.0f, 0.0f };
	cubeVertices[3] = { -0.2f, -0.2f, 0.2f,       255, 255, 0,         0.0f, 0.0f };

	// back
	cubeVertices[4] = { 0.2f, 0.2f, -0.2f,       255, 255, 0,         0.0f, 1.0f };
	cubeVertices[5] = { -0.2f, 0.2f, -0.2f,        255, 255, 0,         1.0f, 1.0f };
	cubeVertices[6] = { -0.2f, -0.2f, -0.2f,       255, 255, 0,         1.0f, 0.0f };
	cubeVertices[7] = { 0.2f, -0.2f, -0.2f,     	255, 255, 0,         0.0f, 0.0f };

	// left
	cubeVertices[8] = { -0.2f, 0.2f, -0.2f,    	  255, 255, 0,         0.0f, 1.0f };
	cubeVertices[9] = { -0.2f, 0.2f, 0.2f,        255, 255, 0,         1.0f, 1.0f };
	cubeVertices[10] = { -0.2f, -0.2f, 0.2f,      255, 255, 0,         1.0f, 0.0f };
	cubeVertices[11] = { -0.2f, -0.2f, -0.2f,     255, 255, 0,         0.0f, 0.0f };

	// right
	cubeVertices[12] = { 0.2f, 0.2f, 0.2f,     		255, 255, 0,         0.0f, 1.0f };
	cubeVertices[13] = { 0.2f, 0.2f, -0.2f,       255, 255, 0,         1.0f, 1.0f };
	cubeVertices[14] = { 0.2f, -0.2f, -0.2f,      255, 255, 0,         1.0f, 0.0f };
	cubeVertices[15] = { 0.2f, -0.2f, 0.2f,      	255, 255, 0,         0.0f, 0.0f };

	// top
	cubeVertices[16] = { -0.2f, 0.2f, -0.2f,      255, 255, 0,         0.0f, 1.0f };
	cubeVertices[17] = { 0.2f, 0.2f, -0.2f,       255, 255, 0,         1.0f, 1.0f };
	cubeVertices[18] = { 0.2f, 0.2f, 0.2f,    	  255, 255, 0,         1.0f, 0.0f };
	cubeVertices[19] = { -0.2f, 0.2f, 0.2f,     	255, 255, 0,         0.0f, 0.0f };

	// bottom
	cubeVertices[20] = { -0.2f, -0.2f, 0.2f,      255, 255, 0,         0.0f, 1.0f };
	cubeVertices[21] = { 0.2f, -0.2f, 0.2f,       255, 255, 0,         1.0f, 1.0f };
	cubeVertices[22] = { 0.2f, -0.2f, -0.2f,      255, 255, 0,         1.0f, 0.0f };
	cubeVertices[23] = { -0.2f, -0.2f, -0.2f,     255, 255, 0,         0.0f, 0.0f };

	// normals
	for(int i = 0; i < 24; i += 4) {
		GLfloat x0 = cubeVertices[i].x;
		GLfloat y0 = cubeVertices[i].y;
		GLfloat z0 = cubeVertices[i].z;

		GLfloat x1 = cubeVertices[i + 1].x;
		GLfloat y1 = cubeVertices[i + 1].y;
		GLfloat z1 = cubeVertices[i + 1].z;
		
		GLfloat x2 = cubeVertices[i + 2].x;
		GLfloat y2 = cubeVertices[i + 2].y;
		GLfloat z2 = cubeVertices[i + 2].z;

		GLfloat v0x = x2 - x0;
		GLfloat v0y = y2 - y0;
		GLfloat v0z = z2 - z0;

		GLfloat v1x = x1 - x0;
		GLfloat v1y = y1 - y0;
		GLfloat v1z = z1 - z0;

		glm::vec3 normal;
		normal = glm::cross(glm::vec3(v0x, v0y, v0z), glm::vec3(v1x, v1y, v1z));

		cubeVertices[i].nx = normal.x;
		cubeVertices[i].ny = normal.y;
		cubeVertices[i].nz = normal.z;
		cubeVertices[i + 1].nx = normal.x;
		cubeVertices[i + 1].ny = normal.y;
		cubeVertices[i + 1].nz = normal.z;
		cubeVertices[i + 2].nx = normal.x;
		cubeVertices[i + 2].ny = normal.y;
		cubeVertices[i + 2].nz = normal.z;
		cubeVertices[i + 3].nx = normal.x;
		cubeVertices[i + 3].ny = normal.y;
		cubeVertices[i + 3].nz = normal.z;
	}

	// // triangular prism
	Vertex triangularPrismVertices[18];
	// front
	triangularPrismVertices[0] = { 0.0f, 0.2f, 0.2f,         255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[1] = { -0.2f, -0.2f, 0.2f,       255, 0, 0,           0.0f, 0.0f };
	triangularPrismVertices[2] = { 0.2f, -0.2f, 0.2f,       255, 0, 0,           1.0f, 0.0f };

	// back
	triangularPrismVertices[3] = { 0.0f, 0.2f, -0.2f,       255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[4] = { -0.2f, -0.2f, -0.2f,     255, 0, 0,           0.0f, 0.0f };
	triangularPrismVertices[5] = { 0.2f, -0.2f, -0.2f,      255, 0, 0,           1.0f, 0.0f };
	
	// top-left side
	triangularPrismVertices[6] = { 0.0f, 0.2f, -0.2f,         255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[7] = { 0.0f, 0.2f, 0.2f,       255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[8] = { -0.2f, -0.2f, 0.2f,       255, 0, 0,           0.0f, 0.0f };
	triangularPrismVertices[9] = { -0.2f, -0.2f, -0.2f,       255, 0, 0,           0.0f, 0.0f };

	// top-right side
	triangularPrismVertices[10] = { 0.0f, 0.2f, 0.2f,         255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[11] = { 0.0f, 0.2f, -0.2f,       255, 0, 0,           0.5f, 1.0f };
	triangularPrismVertices[12] = { 0.2f, -0.2f, -0.2f,       255, 0, 0,           1.0f, 0.0f };
	triangularPrismVertices[13] = { 0.2f, -0.2f, 0.2f,       255, 0, 0,           1.0f, 0.0f };
	
	// bottom
	triangularPrismVertices[14] = { -0.2f, -0.2f, 0.2f,         255, 0, 0,           0.0f, 0.0f };
	triangularPrismVertices[15] = { 0.2f, -0.2f, 0.2f,       255, 0, 0,           1.0f, 0.0f };
	triangularPrismVertices[16] = { 0.2f, -0.2f, -0.2f,       255, 0, 0,           1.0f, 0.0f };
	triangularPrismVertices[17] = { -0.2f, -0.2f, -0.2f,       255, 0, 0,           0.0f, 0.0f };

	for(int i = 0; i < 6; i++) {
		triangularPrismVertices[i].nx = 0.0f;
		triangularPrismVertices[i].ny = 0.0f;
		if(i < 3) {
			triangularPrismVertices[i].nz = 1.0f;
		} else {
			triangularPrismVertices[i].nz = -1.0f;
		}
	}

	for(int i = 6; i < 18; i += 4) {
		GLfloat x0 = triangularPrismVertices[i].x;
		GLfloat y0 = triangularPrismVertices[i].y;
		GLfloat z0 = triangularPrismVertices[i].z;

		GLfloat x1 = triangularPrismVertices[i + 1].x;
		GLfloat y1 = triangularPrismVertices[i + 1].y;
		GLfloat z1 = triangularPrismVertices[i + 1].z;
		
		GLfloat x2 = triangularPrismVertices[i + 2].x;
		GLfloat y2 = triangularPrismVertices[i + 2].y;
		GLfloat z2 = triangularPrismVertices[i + 2].z;

		GLfloat v0x = x2 - x0;
		GLfloat v0y = y2 - y0;
		GLfloat v0z = z2 - z0;

		GLfloat v1x = x1 - x0;
		GLfloat v1y = y1 - y0;
		GLfloat v1z = z1 - z0;

		glm::vec3 normal;
		normal = glm::cross(glm::vec3(v0x, v0y, v0z), glm::vec3(v1x, v1y, v1z));

		triangularPrismVertices[i].nx = normal.x;
		triangularPrismVertices[i].ny = normal.y;
		triangularPrismVertices[i].nz = normal.z;
		triangularPrismVertices[i + 1].nx = normal.x;
		triangularPrismVertices[i + 1].ny = normal.y;
		triangularPrismVertices[i + 1].nz = normal.z;
		triangularPrismVertices[i + 2].nx = normal.x;
		triangularPrismVertices[i + 2].ny = normal.y;
		triangularPrismVertices[i + 2].nz = normal.z;
		triangularPrismVertices[i + 3].nx = normal.x;
		triangularPrismVertices[i + 3].ny = normal.y;
		triangularPrismVertices[i + 3].nz = normal.z;
	}


	//pyramid
	Vertex pyramidVertices[16];

	//triangular #1
	pyramidVertices[0] = { -0.5f, -0.5f, -0.5f,			0, 255, 0,					 0.0f, 0.0f };
	pyramidVertices[1] = { 0.0f, 0.5f, 0.0f,  			0, 255, 0,					 0.5f, 1.0f };
	pyramidVertices[2] = { 0.5f, -0.5f, -0.5f,  			0, 255, 0, 					 1.0f, 0.0f };
	//triangular #2
	pyramidVertices[3] = { 0.5f, -0.5f, -0.5f,  			0, 255, 0, 					 1.0f, 0.0f };
	pyramidVertices[4] = { 0.0f, 0.5f, 0.0f,  			0, 255, 0,					 0.5f, 1.0f };
	pyramidVertices[5] = { 0.5f, -0.5f, 0.5f,  			0, 255, 0,					 1.0f, 1.0f };
	//triangular #3
	pyramidVertices[6] = { 0.5f, -0.5f, 0.5f,  			0, 255, 0,					 1.0f, 1.0f };
	pyramidVertices[7] = { 0.0f, 0.5f, 0.0f,  			0, 255, 0,					 0.5f, 1.0f };
	pyramidVertices[8] = { -0.5f, -0.5f, 0.5f,  			0, 255, 0,					 0.0f, 1.0f };
	//triangular #4
	pyramidVertices[9] = { -0.5f, -0.5f, 0.5f,  			0, 255, 0,					 0.0f, 1.0f };
	pyramidVertices[10] = { 0.0f, 0.5f, 0.0f,  			0, 255, 0,					 0.5f, 1.0f };
	pyramidVertices[11] = { -0.5f, -0.5f, -0.5f,			0, 255, 0,					 0.0f, 0.0f };
	//square base
	pyramidVertices[12] = { -0.5f, -0.5f, -0.5f,			0, 255, 0,					 0.0f, 0.0f };
	pyramidVertices[13] = { 0.5f, -0.5f, -0.5f,  			0, 255, 0, 					 1.0f, 0.0f };
	pyramidVertices[14] = { 0.5f, -0.5f, 0.5f,  			0, 255, 0,					 1.0f, 1.0f };
	pyramidVertices[15] = { -0.5f, -0.5f, 0.5f,  			0, 255, 0,					 0.0f, 1.0f };

	for(int i = 0; i < 12; i+=3) {
		GLfloat x0 = pyramidVertices[i].x;
		GLfloat y0 = pyramidVertices[i].y;
		GLfloat z0 = pyramidVertices[i].z;

		GLfloat x1 = pyramidVertices[i + 1].x;
		GLfloat y1 = pyramidVertices[i + 1].y;
		GLfloat z1 = pyramidVertices[i + 1].z;

		GLfloat x2 = pyramidVertices[i + 2].x;
		GLfloat y2 = pyramidVertices[i + 2].y;
		GLfloat z2 = pyramidVertices[i + 2].z;
		//vector1
		GLfloat v0x = x2 - x0;
		GLfloat v0y = y2 - y0;
		GLfloat v0z = z2 - z0;
		//vector2
		GLfloat v1x = x1 - x0;
		GLfloat v1y = y1 - y0;
		GLfloat v1z = z1 - z0;
		//cross product
		glm::vec3 normal;
		// normal = glm::cross(glm::vec3(v0x, v0y, v0z), glm::vec3(v1x, v1y, v1z));
		normal = glm::cross(glm::vec3(v1x, v1y, v1z), glm::vec3(v0x, v0y, v0z));

		pyramidVertices[i].nx = normal.x;
		pyramidVertices[i].ny = normal.y;
		pyramidVertices[i].nz = normal.z;
		pyramidVertices[i + 1].nx = normal.x;
		pyramidVertices[i + 1].ny = normal.y;
		pyramidVertices[i + 1].nz = normal.z;
		pyramidVertices[i + 2].nx = normal.x;
		pyramidVertices[i + 2].ny = normal.y;
		pyramidVertices[i + 2].nz = normal.z;
	}

	for (int i = 12; i < 16; i++)
	{
		pyramidVertices[i].nx = 0;
		pyramidVertices[i].ny = -1;
		pyramidVertices[i].nz = 0;
	}
	
	// vertex order for EBO
	GLuint cubeIndices[] = {
		0,1,2,    0,2,3,
		4,5,6,    4,6,7,
		8,9,10,   8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23
	};

	GLuint triangularPrismIndices[] = {
		0,1,2, 3,4,5,
		6,7,8, 6,8,9,
		10,11,12, 10,12,13,
		14,15,16, 14,16,17
	};

	GLuint pyramidIndices[] = {
		0,1,2, 3,4,5,
		6,7,8, 9,10,11,
		12,13,14, 12,15,14
	};

	// VBO setup
	GLuint cubeVbo;
	glGenBuffers(1, &cubeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	GLuint triangularPrismVbo;
	glGenBuffers(1, &triangularPrismVbo);
	glBindBuffer(GL_ARRAY_BUFFER, triangularPrismVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangularPrismVertices), triangularPrismVertices, GL_STATIC_DRAW);

	GLuint pyramidVbo;
	glGenBuffers(1, &pyramidVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	// EBO setup
	GLuint cubeEbo;
	glGenBuffers(1, &cubeEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	GLuint triangularPrismEbo;
	glGenBuffers(1, &triangularPrismEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangularPrismEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangularPrismIndices), triangularPrismIndices, GL_STATIC_DRAW);
	
	GLuint pyramidEbo;
	glGenBuffers(1, &pyramidEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// VAO setup
	GLuint cubeVao, triangularPrismVao, pyramidVao;
	glGenVertexArrays(1, &cubeVao);
	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));
	
	glGenVertexArrays(1, &triangularPrismVao);
	glBindVertexArray(triangularPrismVao);
	glBindBuffer(GL_ARRAY_BUFFER, triangularPrismVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	glGenVertexArrays(1, &pyramidVao);
	glBindVertexArray(pyramidVao);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));
	glEnableVertexAttribArray(3);


	glBindVertexArray(0);

	// Create a shader program
	GLuint program = CreateShaderProgram("main.vsh", "main.fsh");

	// Tell OpenGL the dimensions of the region where stuff will be drawn.
	// For now, tell OpenGL to use the whole screen
	glViewport(0, 0, windowWidth, windowHeight);

	GLuint tex;
	glGenTextures(1, &tex);
	// --- Load our image using stb_image ---
	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
	int imageWidth, imageHeight, numChannels;

	// Read the image data and store it in an unsigned char array
	unsigned char* imageData = stbi_load("brickwall.jfif", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	GLuint tex2;
	glGenTextures(1, &tex2);
	imageData = stbi_load("abstract.jpg", &imageWidth, &imageHeight, &numChannels, 0);
	if (imageData != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, tex2);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	glEnable(GL_DEPTH_TEST);

	// LIGHTING
	// point light
	glm::vec3 pointLightPosition(0.0f, 0.0f, 0.0f);
	glm::vec3 pointLightAmbient(1.0f, 0.0f, 2.0f);
	glm::vec3 pointLightDiffuse(0.0f, 2.0f, 1.5f);
	glm::vec3 pointLightSpecular(0.0f, 1.5f, 0.5f);

	// directional light
	glm::vec3 directionalLightDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 directionalLightAmbient(0.0f, 2.0f, 0.0f);
	glm::vec3 directionalLightDiffuse(0.0f, 2.0f, 2.0f);
	glm::vec3 directionalLightSpecular(0.5f, 0.0f, 0.0f);

	// spot light
	glm::vec3 spotLightPosition(-1.0f, 0.0f, -1.0f);
	glm::vec3 spotLightDirection(0.0f, 0.0f, -1.0f);
	glm::vec3 spotLightAmbient(0.0f, 2.0f, 2.0f);
	glm::vec3 spotLightDiffuse(2.0f, 0.0f, 0.0f);
	glm::vec3 spotLightSpecular(0.0f, 0.0f, 9.0f);
	float coneInner(0.1f);
	float coneOuter(0.2f);

	// MATERIALS
	// cube
	glm::vec3 cubeMaterialSpecular(1.0f, 0.0f, 1.0f);
	GLint cubeMaterialLuster(128);

	// triangles
	glm::vec3 triangleMaterialSpecular(1.0f, 1.0f, 0.0f);
	GLint triangleMaterialLuster(1);
	

	GLint cubeIndicesSize = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
	GLint triangularPrismIndicesSize = sizeof(triangularPrismIndices) / sizeof(triangularPrismIndices[0]);
	GLint pyramidIndicesSize = sizeof(pyramidIndices) / sizeof(pyramidIndices[0]);

	// camera and movement
	GLfloat horizontalAngle = M_PI;
	GLfloat verticalAngle = 0.0f;
	GLfloat speed = 4.0f;
	GLfloat mouseSpeed = 0.8f;
	glm::vec3 direction, right, up;

	// starting view position
	glm::vec3 position(0.0f, 0.0f, 1.0f);
	GLdouble xpos, ypos;

	GLfloat lastTime = glfwGetTime();

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentTime = glfwGetTime();
		GLfloat deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		//camera movement
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, (double)windowWidth / 2, (double)windowHeight / 2);

		horizontalAngle += mouseSpeed * deltaTime * float(windowWidth / 2 - xpos);
		verticalAngle += mouseSpeed * deltaTime * float(windowHeight / 2 - ypos);

		direction = glm::vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));
		right = glm::vec3(sin(horizontalAngle - M_PI / 2.0f), 0.0f, cos(horizontalAngle - M_PI / 2.0f));
		up = glm::cross(right, direction);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position += direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position -= direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position += right * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position -= right * deltaTime * speed;
		}

		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shader program that we created
		glUseProgram(program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2);

		GLint texUniformLocation = glGetUniformLocation(program, "tex");

		GLint viewPositionLocation = glGetUniformLocation(program, "viewPosition");
		glUniform3fv(viewPositionLocation, 1, glm::value_ptr(position));

		// point light uniform
		GLint pointLightPositionLocation = glGetUniformLocation(program, "pointLightPosition");
		GLint pointLightAmbientLocation = glGetUniformLocation(program, "pointLightAmbient");
		GLint pointLightDiffuseLocation = glGetUniformLocation(program, "pointLightDiffuse");
		GLint pointLightSpecularLocation = glGetUniformLocation(program, "pointLightSpecular");
		glUniform3fv(pointLightPositionLocation, 1, glm::value_ptr(pointLightPosition));
		glUniform3fv(pointLightAmbientLocation, 1, glm::value_ptr(pointLightAmbient));
		glUniform3fv(pointLightDiffuseLocation, 1, glm::value_ptr(pointLightDiffuse));
		glUniform3fv(pointLightSpecularLocation, 1, glm::value_ptr(pointLightSpecular));

		// directional light uniform
		GLint directionalLightDirectionLocation = glGetUniformLocation(program, "directionalLightDirection");
		GLint directionalLightAmbientLocation = glGetUniformLocation(program, "directionalLightAmbient");
		GLint directionalLightDiffuseLocation = glGetUniformLocation(program, "directionalLightDiffuse");
		GLint directionalLightSpecularLocation = glGetUniformLocation(program, "directionalLightSpecular");
		glUniform3fv(directionalLightDirectionLocation, 1, glm::value_ptr(directionalLightDirection));
		glUniform3fv(directionalLightAmbientLocation, 1, glm::value_ptr(directionalLightAmbient));
		glUniform3fv(directionalLightDiffuseLocation, 1, glm::value_ptr(directionalLightDiffuse));
		glUniform3fv(directionalLightSpecularLocation, 1, glm::value_ptr(directionalLightSpecular));

		// spot light uniform
		GLint spotLightPositionLocation = glGetUniformLocation(program, "spotLightPosition");
		GLint spotLightDirectionLocation = glGetUniformLocation(program, "spotLightDirection");
		GLint spotLightAmbientLocation = glGetUniformLocation(program, "spotLightAmbient");
		GLint spotLightDiffuseLocation = glGetUniformLocation(program, "spotLightDiffuse");
		GLint spotLightSpecularLocation = glGetUniformLocation(program, "spotLightSpecular");
		GLint coneInnerLocation = glGetUniformLocation(program, "coneInner");
		GLint coneOuterLocation = glGetUniformLocation(program, "coneOuter");
		// glUniform3fv(spotLightPositionLocation, 1, glm::value_ptr(spotLightPosition));
		glUniform3fv(spotLightPositionLocation, 1, glm::value_ptr(position));
		// glUniform3fv(spotLightDirectionLocation, 1, glm::value_ptr(spotLightDirection));
		glUniform3fv(spotLightDirectionLocation, 1, glm::value_ptr(direction));
		glUniform3fv(spotLightAmbientLocation, 1, glm::value_ptr(spotLightAmbient));
		glUniform3fv(spotLightDiffuseLocation, 1, glm::value_ptr(spotLightDiffuse));
		glUniform3fv(spotLightSpecularLocation, 1, glm::value_ptr(spotLightSpecular));
		glUniform1f(coneInnerLocation, coneInner);
		glUniform1f(coneOuterLocation, coneOuter);

		// material properties uniform
		GLint materialSpecularLocation = glGetUniformLocation(program, "materialSpecular");
		GLint materialLusterLocation = glGetUniformLocation(program, "materialLuster");

		// identity matrix
		glm::mat4 iMatrix(1.0f);

		// MVP unforms
		glm::mat4 viewMatrix = glm::lookAt(position, position + direction, up);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), windowWidth / windowHeight, 0.1f, 100.0f);

		GLint modelMatrixLocation = glGetUniformLocation(program, "model");
		GLint viewMatrixLocation = glGetUniformLocation(program, "view");
		GLint projectionMatrixLocation = glGetUniformLocation(program, "projection");
		
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));



		// ORDERING:
		// SET VAO
		// SET EBO
		// DO TRANSFORMS
		// SET TRANSFORM UNIFORM
		// SET TEXTURE UNIFORM
		// SET MATERIAL UNIFORM
		// DRAW

		// upper-right cube
		glBindVertexArray(cubeVao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
		glm::mat4 firstMatrix = glm::scale(iMatrix, glm::vec3(1.1f, 1.1f, 1.1f));
		firstMatrix = glm::translate(firstMatrix, glm::vec3(0.5f, 0.3f, -0.2f));
		firstMatrix = glm::rotate(firstMatrix, glm::radians((float)glfwGetTime() * 30), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(firstMatrix));
		glUniform1i(texUniformLocation, 0);
		glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(cubeMaterialSpecular));
		glUniform1i(materialLusterLocation, cubeMaterialLuster);
		glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);

		// // lower-left prism
		glBindVertexArray(triangularPrismVao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangularPrismEbo);
		glm::mat4 secondMatrix = glm::scale(iMatrix, glm::vec3(0.35f, 0.35f, 0.75f));
		secondMatrix = glm::translate(secondMatrix, glm::vec3(-0.9f, -0.9f, -0.4f));
		secondMatrix = glm::rotate(secondMatrix, glm::radians(25.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(secondMatrix));
		glUniform1i(texUniformLocation, 1);
		glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(triangleMaterialSpecular));
		glUniform1i(materialLusterLocation, triangleMaterialLuster);
		glDrawElements(GL_TRIANGLES, triangularPrismIndicesSize, GL_UNSIGNED_INT, 0);

		// // pyramid
		glBindVertexArray(pyramidVao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEbo);
		glUniform1i(texUniformLocation, 0);
		glm::mat4 thirdMatrix = glm::translate(iMatrix, glm::vec3(0.0f, 0.0f, -2.0f));
		thirdMatrix = glm::rotate(thirdMatrix, glm::radians((float)glfwGetTime() * 100), glm::vec3(0.0f, 0.0f, 1.0f));
		thirdMatrix = glm::rotate(thirdMatrix, glm::radians((float)glfwGetTime() * 150), glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(thirdMatrix));
		glDrawElements(GL_TRIANGLES, pyramidIndicesSize, GL_UNSIGNED_INT, 0);

		// upper-left cube
		glBindVertexArray(cubeVao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEbo);
		glm::mat4 fourthMatrix = glm::scale(iMatrix, glm::vec3(0.9f, 0.9f, 0.9f));
		fourthMatrix = glm::translate(fourthMatrix, glm::vec3(-0.5f, 0.5f, -0.7f));
		fourthMatrix = glm::rotate(fourthMatrix, glm::radians(75.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(fourthMatrix));
		glUniform1i(texUniformLocation, 1);
		glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(cubeMaterialSpecular));
		glUniform1i(materialLusterLocation, cubeMaterialLuster);
		glDrawElements(GL_TRIANGLES, cubeIndicesSize, GL_UNSIGNED_INT, 0);

		// lower-right prism
		glBindVertexArray(triangularPrismVao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangularPrismEbo);
		glm::mat4 fifthMatrix = glm::scale(iMatrix, glm::vec3(0.7f, 0.7f, 0.7f));
		fifthMatrix = glm::translate(fifthMatrix, glm::vec3(0.4f, -0.5f, -0.2f));
		fifthMatrix = glm::rotate(fifthMatrix, glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(fifthMatrix));
		glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(triangleMaterialSpecular));
		glUniform1i(materialLusterLocation, triangleMaterialLuster);
		glDrawElements(GL_TRIANGLES, triangularPrismIndicesSize, GL_UNSIGNED_INT, 0);

		// "Unuse" the vertex array object
		glBindVertexArray(0);

		// Tell GLFW to swap the screen buffer with the offscreen buffer
		glfwSwapBuffers(window);

		// Tell GLFW to process window events (e.g., input events, window closed events, etc.)
		glfwPollEvents();
	}

	// --- Cleanup ---

	// Make sure to delete the shader program
	glDeleteProgram(program);

	// Delete the VBOs that contain our vertices
	glDeleteBuffers(1, &cubeVbo);
	glDeleteBuffers(1, &triangularPrismVbo);
	glDeleteBuffers(1, &pyramidVbo);

	// Delete the vertex array objects
	glDeleteVertexArrays(1, &cubeVao);
	glDeleteVertexArrays(1, &triangularPrismVao);
	glDeleteVertexArrays(1, &pyramidVao);

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
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
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "program link error: " << infoLog << std::endl;
	}

	return program;
}

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
{
	std::ifstream shaderFile(shaderFilePath);
	if (shaderFile.fail())
	{
		std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
		return 0;
	}

	std::string shaderSource;
	std::string temp;
	while (std::getline(shaderFile, temp))
	{
		shaderSource += temp + "\n";
	}
	shaderFile.close();

	return CreateShaderFromSource(shaderType, shaderSource);
}

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
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
	if (compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "shader compilation error: " << infoLog << std::endl;
	}

	return shader;
}

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}
