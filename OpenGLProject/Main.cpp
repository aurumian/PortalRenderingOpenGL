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

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

static const char* vertexShaderSource = "#version 330 core \n layout (location = 0) in vec3 aPos; \n\n void main(){ gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); }";

static const char* fragmentShaderSource = "#version 330 core \n out vec4 FragColor; \n uniform vec4 ourColor; void main() {FragColor = ourColor; }";

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

	//define vertices to draw
	float vertices[] = {
		 -0.75f,  0.0f, 0.0f,
		 -0.5f, 0.5f, 0.0f,
		 -0.25f, 0.0f, 0.0f
	};
	float vertices2[] = {
		  //positions			//colors
		  0.25f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		  0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,
		  0.75f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	unsigned int VBO;
	glGenBuffers(1, &VBO);


	//compile vertex shader
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	//verify shader compiled
	int success;
	const int infoLogSize = 1024;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[infoLogSize];
		glGetShaderInfoLog(vertexShader, infoLogSize, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//compile fragment shader
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	//verify shader compiled
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[infoLogSize];
		glGetShaderInfoLog(fragmentShader, infoLogSize, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//create shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[infoLogSize];
		glGetProgramInfoLog(shaderProgram, infoLogSize, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	//tell openGl how to interprete input data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	//clean up
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//create and configure second vertex array object
	GLuint vao2;
	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	GLuint vbo2;
	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//create shaderProgram
	Shader sp = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	sp.use();
	sp.setVec3("offset", 0.2f, 0.0f, 0.0f);

	//render loop
	while (!glfwWindowShouldClose(window)) {
		//input
		processInput(window);

		//rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		float time = glfwGetTime();
		float blueVal = (sin(time) + 1.0f) * 0.5f;
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		glUseProgram(shaderProgram);
		glUniform4f(vertexColorLocation, 0.0f, 1.0f, blueVal, 1.0f);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		sp.use();
		glBindVertexArray(vao2);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);

		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clean glfw resources
	glfwTerminate();
	return 0;
}

