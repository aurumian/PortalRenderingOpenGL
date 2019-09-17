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

// color values
Transform bulbTransform;
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

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
	GLuint diffuseMap;
	glGenTextures(1, &diffuseMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	// set texture wrapping/filtering options (on the currently bound object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("C:\\Users\\123\\Desktop\\container2.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(diffuseMap);
	}
	else {
		std::cout << "Failed to load texture" << endl;
	}
	// free memory
	stbi_image_free(data);

	// generate second texture object
	GLuint specularMap;
	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("C:\\Users\\123\\Desktop\\container2spec.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		std::cout << "Failed to load texture2" << endl;
	}
	// free memory
	stbi_image_free(data);

	//define vertices to draw
	float vertices[] = {
	//position			  //normals				//texture coordinates
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,	0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,	0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,	1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,	1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,	0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,	1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,	0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,	0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,	0.0f, 1.0f
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

	// positions all containers
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
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 4. Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	// 5. enable vertex attribute arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// create bulbVAO
	GLuint bulbVAO;
	glGenVertexArrays(1, &bulbVAO);
	glBindVertexArray(bulbVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	//create shaderProgram
	Shader sp = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	sp.use();

	// create bulb shader program
	Shader bulbSP = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbVertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbFragmentShader.fsf");

	// initialize mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	oldMouseX = (float)mouseX;
	oldMouseY = (float)mouseY;

	// set light position
	bulbTransform.position = glm::vec3(2.0f, 2.0f, 5.0f);
	bulbTransform.rotation.SetEulerAngles(glm::vec3(0.0f, 0.0f, 45.0f));
	bulbTransform.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

	{
		glm::vec3 test = glm::mat3(camera.GetWorldToViewMatrix()) * glm::vec3(0.0f, -1.0f, 0.0f);
		cout << "viewspace light dir: " << test.x << " " << test.y << " " << test.z << endl;
	}

	// set camera position
	{
		Transform t = camera.GetTransform();
		t.position = glm::vec3(0.0f, 0.0f, -3.0f);
		camera.SetTransform(t);
	}


	//open gl render config
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		
		//input
		processInput(window, &sp);

		//rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		sp.use();

		// set uniform matricies
		glm::mat4 worldToView;
		glm::mat4 projection;
		worldToView = camera.GetWorldToViewMatrix();
		projection = camera.GetProjectionMatrix();
		sp.setMat4("worldToView", worldToView);
		sp.setMat4("projection", projection);
		// set light properties
		//lightColor.x = sin(glfwGetTime() * 2.0f);
		//lightColor.y = sin(glfwGetTime() * 0.7f);
		//lightColor.z = sin(glfwGetTime() * 1.3f);
		sp.setVec3("dirLight.color", lightColor * 0.5f + 0.5f);
		sp.setFloat("dirLight.intensity", 1.0f);
		sp.setVec3("dirLight.direction", glm::mat3(worldToView) * glm::vec3(0.0f, -1.0f, 0.0f));
		sp.setFloat("dirLight.ambientStrength", 0.2f);
		sp.setVec3("pointLight.position", worldToView * glm::vec4(bulbTransform.position, 1.0f));


		// set material properties
		sp.setInt("material.diffuse", 0);
		sp.setInt("material.specular", 1);
		sp.setFloat("material.shiness", 32.0f);

		// bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);



		glBindVertexArray(VAO);
		for (int i = 0; i < 10; i++) {
			Transform transform;
			transform.rotation.SetEulerAngles(glm::vec3(i* 20.0f, i*45.0f, 0.0f));
			transform.position = cubePositions[i];
			//transform.SetScale(glm::vec3(0.75f, 2.5f, 1.5f));

			glm::mat4 objectToWorld = transform.GetObjectToWorldMatrix();
			sp.setMat4("objectToWorld", objectToWorld);
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
			sp.SetMat3("normalMatrix", normalMatrix);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		bulbSP.use();
		// set uniform values
		bulbSP.setMat4("worldToView", worldToView);
		bulbSP.setMat4("projection", projection);
		bulbSP.setVec3("lightColor", lightColor);
		glBindVertexArray(bulbVAO);
		bulbSP.setMat4("objectToWorld", bulbTransform.GetObjectToWorldMatrix());
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clean glfw resources
	glfwTerminate();
	return 0;
}

