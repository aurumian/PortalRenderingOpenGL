#define GLM_FORCE_LEFT_HANDED

#include <string>
#include <iostream>

#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Time.h"
#include "Shader.h"
#include "Rotator.h"
#include "FPSCameraController.h"

Camera camera;
FPSCameraController fpsCam = FPSCameraController(&camera);

static int screenWidth = 800, screenHeight = 600;
/* window resize callback
 * tell OpenGL that the rendering window size changed
*/
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	screenWidth = width;
	screenHeight = height;
	glViewport(0, 0, width, height);
	camera.SetAspectRatio((float)width / (float)height);
}

static glm::vec3 eul = glm::vec3(0, 0, 0);




static float oldMouseX;
static float oldMouseY;
void processInput(GLFWwindow* window, Shader* shader = NULL) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	fpsCam.ProcessInput(window,(float)mouseX - oldMouseX, (float)mouseY - oldMouseY);
	oldMouseX = (float)mouseX;
	oldMouseY = (float)mouseY;
	
	if (shader) {
		const float alphaDelta = 0.005f;
		float newAlpha = shader->getFloat("mixAlpha");
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			newAlpha += alphaDelta;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			newAlpha -= alphaDelta;

		if (newAlpha < 0.0f)
			newAlpha = 0.0f;
		else if (newAlpha > 1.0f)
			newAlpha = 1.0f;

		shader->setFloat("mixAlpha", newAlpha);
	}
}

int main() {

	//initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//initialze glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialie GLAD" << std::endl;
		return -1;
	}

	//register window resize callback
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	//tell OpenGL the size of the rendering window
	glViewport(0, 0, screenWidth, screenHeight);

	camera.SetAspectRatio((float)screenWidth / (float)screenHeight);

	// generate texture object
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set texture wrapping/filtering options (on the currently bound object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("C:\\Users\\123\\Desktop\\AlchemixBefore.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(texture);
	}
	else {
		cout << "Failed to load texture" << endl;
	}
	// fre memory
	stbi_image_free(data);

	// generate second texture object
	GLuint texture1;
	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set texture wrapping/filtering options (on the currently bound object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("C:\\Users\\123\\Desktop\\steelTexture.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(texture);
	}
	else {
		cout << "Failed to load texture" << endl;
	}
	// fre memory
	stbi_image_free(data);

	//define vertices to draw
	float vertices[] = {
		  //positions				//colors				// texture coords
		  0.5f, 0.5f, 0.5f,			1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		// front top rigth
		  0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		// front bottom right
		  -0.5f, -0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		// front bottom left
		  -0.5f, 0.5f, 0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,		// front top left
		   //positions				//colors				// texture coords
		  0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		// back top rigth
		  0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		// back bottom right
		  -0.5f, -0.5f, -0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		// back bottom left
		  -0.5f, 0.5f, -0.5f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f		// back top left
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3,
		2, 6, 3,
		7, 3, 6,
		7, 4, 5,
		7, 6, 5,
		0, 4, 5,
		0, 1, 5,
		3, 0, 4,
		3, 7, 4,
		2, 1, 5,
		2, 6, 5
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, 15.0f),
		glm::vec3(-1.5f, -2.2f, 2.5f),
		glm::vec3(-3.8f, -2.0f, 12.3f),
		glm::vec3(2.4f, -0.4f, 3.5f),
		glm::vec3(-1.7f,  3.0f, 7.5f),
		glm::vec3(1.3f, -2.0f, 2.5f),
		glm::vec3(1.5f,  2.0f, 2.5f),
		glm::vec3(1.5f,  0.2f, 1.5f),
		glm::vec3(-1.3f,  1.0f, 1.5f)
	};

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	//create element buffer object
	GLuint EBO;
	glGenBuffers(1, &EBO);

	//create and configure Vertex Array Object
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	// 1. bind vao
	glBindVertexArray(VAO);
	// 2. copy verticies array into a buffer for OpenGl to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 3. copy index array into an elements buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 4. Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//create shaderProgram
	Shader sp = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	sp.use();
	sp.setInt("texture1", 0);
	sp.setInt("texture2", 1);

	
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	oldMouseX = (float)mouseX;
	oldMouseY = (float)mouseY;

	glEnable(GL_DEPTH_TEST);

	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		
		//input
		processInput(window, &sp);

		//rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sp.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glm::mat4 worldToView;
		glm::mat4 projection;

		worldToView = camera.GetWorldToViewMatrix();
		projection = camera.GetProjectionMatrix();
	

		// set uniform matricies
		sp.setUniform("worldToView", worldToView);
		sp.setUniform("projection", projection);

		glBindVertexArray(VAO);
		for (int i = 0; i < 10; i++) {
			
			glm::mat4 objectToWorld = glm::mat4(1.0f);
			objectToWorld = glm::translate(objectToWorld, cubePositions[i]);
			if (i % 3 == 0)
				objectToWorld = glm::rotate(objectToWorld, (float)glfwGetTime() * glm::radians(52.0f), glm::normalize(glm::vec3(0.5f, 1.0f, 0.0f)));
			else
				objectToWorld = glm::rotate(objectToWorld, -i * glm::radians(52.0f), glm::normalize(glm::vec3(0.5f, 1.0f, 0.0f)));
			// set uniform matricies
			sp.setUniform("objectToWorld", objectToWorld);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(0);

		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clean glfw resources
	glfwTerminate();
	return 0;
}

