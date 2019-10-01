#define GLM_FORCE_LEFT_HANDED

#include <string>
#include <iostream>
#include <algorithm>

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
#include "Mesh.h"

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
	Texture diffTex;
	diffTex.type = "texture_diffuse";
	glGenTextures(1, &diffTex.id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffTex.id);
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
		glGenerateMipmap(diffTex.id);
	}
	else {
		std::cout << "Failed to load texture" << endl;
	}
	// free memory
	stbi_image_free(data);

	// generate second texture object
	Texture specTex;
	specTex.type = "texture_specular";
	glGenTextures(1, &specTex.id);
	glBindTexture(GL_TEXTURE_2D, specTex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("C:\\Users\\123\\Desktop\\container2spec.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(specTex.id);
	}
	else {
		std::cout << "Failed to load texture2" << endl;
	}
	// free memory
	stbi_image_free(data);

	// generate grass texture object
	Texture grassTex;
	grassTex.type = "texture_diffuse";
	glGenTextures(1, &grassTex.id);
	glBindTexture(GL_TEXTURE_2D, grassTex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("D:\\MyFiles\\SomeStuff\\grass.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(grassTex.id);
	}
	else {
		std::cout << "Failed to load texture2" << endl;
	}
	// free memory
	stbi_image_free(data);

	// generate grass texture object
	Texture windowTex;
	grassTex.type = "texture_diffuse";
	glGenTextures(1, &windowTex.id);
	glBindTexture(GL_TEXTURE_2D, windowTex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("D:\\MyFiles\\SomeStuff\\window.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(windowTex.id);
	}
	else {
		std::cout << "Failed to load texture2" << endl;
	}
	// free memory
	stbi_image_free(data);

	//define vertices to draw
	float vertices[] = {
	//position			  //normals			//texture coordinates
	// Front face
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
	0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
	0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// Back face
	-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
	0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
	0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
	0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
	// Left face
	-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
	-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
	-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
	// Right face
	0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
	0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
	0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
	0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
	// Bottom face
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
	0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
	0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
	0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
	// Top face
	-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
	0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
	0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
	0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left        
	};
	unsigned int indices[] = {
		0,	1,	2,
		3,	4,	5,
		6,	7,	8,
		9,	10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35
	};

	Vertex* v = (Vertex*)vertices;
	vector<Vertex> verts(v, v+36);
	vector<GLuint> inds(indices, indices + 36);
	vector<Texture> texs;
	texs.push_back(diffTex);
	texs.push_back(specTex);
	Mesh container = Mesh(verts, inds, texs);

	// create flloor mesh
	verts.clear();
	verts.push_back({glm::vec3(1, 0, -1), glm::vec3(0,1,0), glm::vec2(1, 1)});
	verts.push_back({glm::vec3(1, 0, 1), glm::vec3(0,1,0), glm::vec2(1, 0)});
	verts.push_back({glm::vec3(-1, 0, 1), glm::vec3(0,1,0), glm::vec2(0, 0)});
	verts.push_back({glm::vec3(-1, 0, -1), glm::vec3(0,1,0), glm::vec2(0, 1)});
	inds.clear();
	inds.push_back(0);
	inds.push_back(1);
	inds.push_back(2);
	inds.push_back(2);
	inds.push_back(3);
	inds.push_back(0);
	Mesh floor = Mesh(verts, inds, texs);

	// create grass mesh
	verts.clear();
	verts.push_back({ glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 0) });
	verts.push_back({ glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 1) });
	verts.push_back({ glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 1) });
	verts.push_back({ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 0) });
	texs.clear();
	texs.push_back(grassTex);
	Mesh grass = Mesh(verts, inds, texs);

	// create window mesh
	texs.clear();
	texs.push_back(windowTex);
	Mesh windowMesh = Mesh(verts, inds, texs);

	// positions of all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  3.0f,  0.0f),
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

	// positions of pieces of grass
	glm::vec3 grassPositions[] = {
		glm::vec3(0.2f, 0.0f, 0.1f),
		glm::vec3(0.25f, 0.0f, 0.19f),
		glm::vec3(-0.25f, 0.0f, 0.19f),
		glm::vec3(-0.25f, 0.0f, -0.19f),
		glm::vec3(-0.5f, 0.0f, -0.34f),
		glm::vec3(-0.5f, 0.0f, 0.34f),
		glm::vec3(0.5f, 0.0f, 0.34f),
		glm::vec3(0.5f, 0.0f, 0.34f)
	};
	glm::vec3 windowPositions[] = {
		glm::vec3(0, 0, 2.0f),
		glm::vec3(0.5, 0, 1.0f),
		glm::vec3(0.25f, 0, -1.0f),
		glm::vec3(-0.25f, 0, 1.5f),
	};

	//create shaderProgram
	Shader sp = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");

	// create bulb shader program
	Shader bulbSP = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbVertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbFragmentShader.fsf");

	// create grass shader program
	Shader grassShader = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\GrassFragmentShader.fsf");

	// create window shader program
	Shader windowShader = Shader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs", "D:\\VSProjects\\OpenGLProject\\OpenGLProject\\WindowFragmentShader.fsf");

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

	// set initial camera position
	{
		Transform t = camera.GetTransform();
		t.position = glm::vec3(0.0f, 0.0f, -3.0f);
		camera.SetTransform(t);
	}

	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		
		//input
		processInput(window, &sp);

		//rendering
		//open gl render config
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
		sp.setFloat("material.shiness", 32.0f);

		// draw crates
		for (int i = 0; i < 10; i++) {
			Transform transform;
			transform.rotation.SetEulerAngles(glm::vec3(i* 20.0f, i*45.0f, 0.0f));
			transform.position = cubePositions[i];
			//transform.SetScale(glm::vec3(0.75f, 2.5f, 1.5f));

			glm::mat4 objectToWorld = transform.GetTransformMatrix();
			sp.setMat4("objectToWorld", objectToWorld);
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
			sp.SetMat3("normalMatrix", normalMatrix);
			container.Draw(sp);
		}

		// draw floor
		Transform transform;
		transform.position = glm::vec3(0, -0.5f, 3);
		transform.SetScale(glm::vec3(10));
		glm::mat4 objectToWorld = transform.GetTransformMatrix();
		sp.setMat4("objectToWorld", objectToWorld);
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
		sp.SetMat3("normalMatrix", normalMatrix);
		floor.Draw(sp);

		// draw grass
		grassShader.use();
		grassShader.setMat4("worldToView", worldToView);
		grassShader.setMat4("projection", projection);
		for (int i = 0; i < 8; i++) {
			Transform transform;
			transform.rotation.SetEulerAngles(glm::vec3(0.0f , i*17.5f, 0.0f));
			transform.position = grassPositions[i];

			glm::mat4 objectToWorld = transform.GetTransformMatrix();
			grassShader.setMat4("objectToWorld", objectToWorld);
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
			grassShader.SetMat3("normalMatrix", normalMatrix);
			grass.Draw(grassShader);
		}

		// draw bulb
		bulbSP.use();
		// set uniform values
		bulbSP.setMat4("worldToView", worldToView);
		bulbSP.setMat4("projection", projection);
		bulbSP.setVec3("lightColor", lightColor);
		bulbSP.setMat4("objectToWorld", bulbTransform.GetTransformMatrix());
		container.Draw(bulbSP);

		// draw windows
		windowShader.use();
		windowShader.setMat4("worldToView", worldToView);
		windowShader.setMat4("projection", projection);
		// sort the positions to draw the farthest first
		sort(windowPositions, windowPositions + 4,
			[](const glm::vec3& x, const glm::vec3& y) {
			float xDist = glm::length(camera.GetTransform().position - x);
			float yDist = glm::length(camera.GetTransform().position - y);
			return xDist > yDist;
			}
		);
		for (int i = 0; i < 4; i++) {
			Transform transform;
			//transform.rotation.SetEulerAngles(glm::vec3(0.0f, i*17.5f, 0.0f));
			transform.position = windowPositions[i];

			glm::mat4 objectToWorld = transform.GetTransformMatrix();
			windowShader.setMat4("objectToWorld", objectToWorld);
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
			windowShader.SetMat3("normalMatrix", normalMatrix);
			windowMesh.Draw(windowShader);
		}
		
		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//clean glfw resources
	glfwTerminate();
	return 0;
}

