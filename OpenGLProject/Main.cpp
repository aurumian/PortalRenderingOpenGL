#include <string>
#include <iostream>
#include "ShaderProgram.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;
float mix = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void PprocessInput(GLFWwindow* window, ShaderProgram shaderProgram)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		mix += 0.01f;
		if (mix > 1.0f) mix = 1.0f;
		shaderProgram.setUniformValue("mix", mix);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		mix -= 0.01f;
		if (mix < 0.0f) mix = 0.0f;
		shaderProgram.setUniformValue("mix", mix);
	}

}

//my shape
float vertices[] = { // positions // colors // texture coords 
	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right 
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right 
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left 
	-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left 
};

float texCoords[]{
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.5f, 1.0f
};


unsigned int indicies[] = {
	0, 1, 3, //first triangle
	1, 2, 3 //second triangle
};

/*Vertex Shader*/
std::string vertexShaderCode = "#version 330 core \nlayout(location = 0) in vec3 aPos; layout(location = 1) in vec3 aColor; out vec3 ourColor;  void main() {gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); ourColor = aColor;}";



int main() {
	/*Initialize and configure glfw*/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/*make sure pointers to opengl functions are initialized*/
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

	glViewport(0, 0, 800, 600);

	ShaderProgram shaderProgram("D:/VSProjects/OpenGLProject/OpenGLProject/VertexShader.vs", "D:/VSProjects/OpenGLProject/OpenGLProject/FragmenShader.fs");

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	//Initialization
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
	/*tell OpenGL how to read vertex data*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float)*3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);

	
	//Generate texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//texture config
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//load image
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrChannels;
		unsigned char* data = stbi_load("C:\\Users\\123\\Desktop\\image1.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	//Generate texture2
	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	//texture2 config
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	{
		//stbi_set_flip_vertically_on_load(true);
		int width, height, nrChannels;
		unsigned char* data = stbi_load("C:\\Users\\123\\Desktop\\RobinHead.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}

	shaderProgram.use();
	shaderProgram.setUniformValue("ourTexture1", 0);
	shaderProgram.setUniformValue("ourTexture2", 1);
	shaderProgram.setUniformValue("mix", mix);
	
	/*Main Loop*/
	while (!glfwWindowShouldClose(window))
	{
		PprocessInput(window, shaderProgram);


		//rendering commands
		glClearColor(0.0f, 0.2f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderProgram.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		//check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}