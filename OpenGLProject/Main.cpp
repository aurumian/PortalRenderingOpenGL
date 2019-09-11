#include <string>
#include <iostream>
#include "Shader.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/* window resize callback
 * tell OpenGL that the rendering window size changed
*/
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, Shader* shader = NULL) {
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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

	//tell OpenGL the size of the rendering window
	glViewport(0, 0, 800, 600);
	//register window resize callback
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

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
		  0.5f, 0.5f, 0.0f,			1.0f, 0.0f, 0.0f,		1.0f, 1.0f,		// top rigth
		  0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,		// bottom right
		  -0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		// bottom left
		  -0.5f, 0.5f, 0.0f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f		// top left
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
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

	//render loop
	while (!glfwWindowShouldClose(window)) {
		//input
		processInput(window, &sp);

		//rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sp.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clean glfw resources
	glfwTerminate();
	return 0;
}

