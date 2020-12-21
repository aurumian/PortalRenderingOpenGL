

#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <list>
#include <map>

#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MathInclude.h"

#include "Time.h"
#include "Shader.h"
#include "Rotator.h"
#include "PlayerController.h"
#include "Camera.h"
#include "Mesh.h"
#include "Physics.h"
#include "MeshRenderer.h"
#include "MeshLoader.h"
#include "ShaderCompiler.h"
#include "Material.h"
#include "Portal.h"

using namespace std;

const int MAX_PORTAL_DEPTH = 2;

void DrawScene();
/*@pre Stencil testing must be enabled
  @pre Stencil buffer should be cleared with 0's
 */
void PrepareDrawPortal(const Portal& p, const glm::mat4& worldToView);
glm::mat4 DrawPortal(const Portal& p1, const Portal& p2, const glm::mat4& worldToView);

void UpdateStencil(const list<Portal*>& pl, unordered_map<const Portal*, glm::mat4>& wtvs);

void OnPlayerTriggerEnter(const RayHit& hit);

Camera camera;
Player player(&camera);
PlayerController pc(&player);
Physics physics;

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

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		cout << camera.GetTransform().ToString() << endl;
	}

	
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	pc.ProcessInput(window,(float)mouseX - oldMouseX, (float)mouseY - oldMouseY);
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


glm::mat4 worldToView;
glm::mat4 projection;

// Renderers
MeshRenderer* crateRenderer;
MeshRenderer* windowRenderer;
MeshRenderer* floorRenderer;
MeshRenderer* grassRenderer;
MeshRenderer* bulbRenderer;
MeshRenderer* monkeyRenderer;
MeshRenderer* sceneRenderer;
MeshRenderer* scene2Renderer;

// Portals
Portal p1;
Portal p2;
Portal p3;
Portal p4;
Portal p5;
Portal p6;
Portal p7;
Portal p8;


int main() {
	string s;
	cin >> s;
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
	Texture scene1Tex;
	scene1Tex.type = "texture_diffuse";
	glGenTextures(1, &scene1Tex.id);
	glBindTexture(GL_TEXTURE_2D, scene1Tex.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\scene_tex.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(scene1Tex.id);
	}
	else {
		std::cout << "Failed to load texture2" << endl;
	}
	// free memory
	stbi_image_free(data);

	// generate grass texture object
	Texture windowTex;
	scene1Tex.type = "texture_diffuse";
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
	Material contMat;
	contMat.textures = texs;
	Mesh container = Mesh(verts, inds);

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
	Mesh floor = Mesh(verts, inds);

	// create portal plane mesh
	glm::vec2 portalDims = glm::vec2(0.55f, 1.2f );
	verts.clear();
	verts.push_back({ glm::vec3(portalDims.x, -portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 0) });
	verts.push_back({ glm::vec3(portalDims.x, portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 1) });
	verts.push_back({ glm::vec3(-portalDims.x, portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 1) });
	verts.push_back({ glm::vec3(-portalDims.x, -portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 0) });
	texs.clear();
	texs.push_back(scene1Tex);
	Material sceneMat;
	sceneMat.textures = texs;
	Mesh portalPlane = Mesh(verts, inds);

	// create window mesh
	texs.clear();
	texs.push_back(windowTex);
	Material windowMat;
	windowMat.textures = texs;
	Mesh windowMesh = Mesh(verts, inds);

	ShaderCompiler comp;
	//create shaderProgram
	comp.AddFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	comp.AddVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* sp = comp.Compile();

	// create bulb shader program
	comp.AddFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbFragmentShader.fsf");
	comp.AddVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* bulbSP = comp.Compile();

	// create grass shader program
	comp.AddFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\GrassFragmentShader.fsf");
	comp.AddVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* grassShader = comp.Compile();

	// create window shader program
	comp.AddVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	comp.AddFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\WindowFragmentShader.fsf");
	Shader* windowShader = comp.Compile();

	contMat.shader = sp;
	sceneMat.shader = grassShader;
	windowMat.shader = windowShader;

	MeshLoader ml;
	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\monkey.fbx");
	Mesh monkey = ml.GetMesh(0);

	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\portal_scene.fbx");
	Mesh portalScene = ml.GetMesh(1);

	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\portal_scene2.fbx");
	Mesh portalScene2 = ml.GetMesh(0);

	sceneMat.shader = sp;

	// Create renderers
	crateRenderer = new MeshRenderer(&container, &contMat);
	floorRenderer = new MeshRenderer(&floor, &contMat);
	grassRenderer = new MeshRenderer(&portalPlane, &sceneMat);
	bulbRenderer = new MeshRenderer(&container, new Material(bulbSP));
	windowRenderer = new MeshRenderer(&windowMesh, &windowMat);
	monkeyRenderer = new MeshRenderer(&monkey, &contMat);
	sceneRenderer = new MeshRenderer(&portalScene, &sceneMat);
	scene2Renderer = new MeshRenderer(&portalScene2, &sceneMat);

	vector<Portal*> portals;
	portals.push_back(&p1);
	portals.push_back(&p2);
	portals.push_back(&p3);
	portals.push_back(&p4);
	portals.push_back(&p5);
	portals.push_back(&p6);
	portals.push_back(&p7);
	portals.push_back(&p8);
	for (Portal* p : portals) {
		p->AddComponent(new MeshRenderer(&windowMesh, new Material(bulbSP)));
	}

	// Create portals
	{
		
		// portal 1
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, 54.15f };
			t.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			p1.transform = t;
			p1.stencilVal = 1;
			p1.dest = &p3;
			p1.SetMaxRenderDepth(3);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p1.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}
		
		// portal 2
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, 35.5f };
			t.rotation = Rotator({ 0.0f, 180.0f, 0.0f });
			p2.transform = t;
			p2.stencilVal = 2;
			p2.dest = &p4;
			p2.SetMaxRenderDepth(3);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p2.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 3
		{
			Transform t;
			t.position = glm::vec3(0.0f, 1.25f, 2.1f);
			t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
			p3.transform = t;
			p3.stencilVal = 3;
			p3.dest = &p1;
			p3.cbsPortals.push_back(&p2);
			p3.SetMaxRenderDepth(3);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p3.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 4
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, -2.1f };
			t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
			p4.transform = t;
			p4.stencilVal = 4;
			p4.dest = &p2;
			p4.SetMaxRenderDepth(3);
			p4.cbsPortals.push_back(&p1);
			//p4.cbsPortals.push_back(&p8);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p4.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 5
		{
			Transform t;
			t.position = { 2.1f, portalDims.y, 0.0f };
			t.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
			p5.transform = t;
			p5.stencilVal = 1;
			p5.dest = &p7;
			p5.SetMaxRenderDepth(3);
			//p5.cbsPortals.push_back(&p8);
			//p5.cbsPortals.push_back(&p1);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p5.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 6
		{
			Transform t;
			t.position = { 15.0f, portalDims.y, 50.0f };
			t.rotation = glm::vec3(0.0f, -90.0f, 0.0f);
			p6.transform = t;
			p6.stencilVal = 1;
			p6.dest = &p5;
			p6.SetMaxRenderDepth(3);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p6.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 7
		{
			Transform t;
			t.position = {-2.1f, portalDims.y, 0.0f };
			t.rotation = glm::vec3(0.0f, -90.0f, 0.0f);
			p7.transform = t;
			p7.stencilVal = 1;
			p7.dest = &p5;
			p7.SetMaxRenderDepth(3);
			//p7.cbsPortals.push_back(&p6);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p7.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}

		// portal 8
		{
			Transform t;
			t.position = { -4.2f, portalDims.y, 50.0f };
			t.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
			p8.transform = t;
			p8.stencilVal = 1;
			p8.dest = &p7;
			p8.SetMaxRenderDepth(3);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p8.AddComponent(col);
			Physics::Instance()->AddCollider(col);
		}
	}

	// initialize mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	oldMouseX = (float)mouseX;
	oldMouseY = (float)mouseY;

	// set light position
	bulbTransform.position = glm::vec3(30.0f, 1.0f, 3.0f);
	bulbTransform.rotation.SetEulerAngles(glm::vec3(0.0f, 0.0f, 45.0f));
	bulbTransform.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

	// set initial camera position
	{
		Transform t = camera.GetTransform();
		t.position = glm::vec3(0.0f, 0.75, -3.0f);
		camera.SetTransform(t);
	}

	// 
	player.onTriggerHitCallback = OnPlayerTriggerEnter;

	// prepare a PortalRenderTree for easier portal rendering
	// it also optimizes calculations since it precomputes portalling matricies
	list<Portal*> pl;
	pl.push_back(&p1);
	pl.push_back(&p2);
	pl.push_back(&p3);
	pl.push_back(&p4);
	pl.push_back(&p5);
	pl.push_back(&p7);
	PortalRenderTree tree = Portal::GetPortalRenderTree(pl);

	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		
		//input
		processInput(window, sp);

		//rendering
		{
			//open gl render config
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// draw the scene the first time
			worldToView = camera.GetWorldToViewMatrix();
			projection = camera.GetProjectionMatrix();
			DrawScene();

			// divide stencil range by portals
			{
				vector<Portal*> pv;
				pv.push_back(&p1);
				pv.push_back(&p2);
				pv.push_back(&p3);
				pv.push_back(&p4);
				pv.push_back(&p5);
				//pv.push_back(&p6);
				pv.push_back(&p7);
				//pv.push_back(&p8);
				struct Helper {
					Portal* portal;
					list<Portal*> pq;
					size_t count = 0;
				};
				size_t total = 0;
				size_t depth = 0;
				vector<Helper> hv;
				for (Portal* p : pv) {
					list<Portal*> q;
					q.push_front(p);
					hv.push_back({ p, q, 0 });
				}

				bool done = false;
				size_t numEmpty = 0;
				while (!done) {
					depth += 1;
					for (Helper& h : hv) {
						size_t len = h.pq.size();
						if (len == 0)
							numEmpty++;
						if (numEmpty == hv.size())
							done = true;
						for (size_t i = 0; i < len && !done; i++) {
							Portal* p = h.pq.front();
							h.pq.pop_front();
							total++;
							h.count++;
							if (total >= 255) {
								done = true;
								break;
							}
							//if (depth >= p->GetMaxRenderDepth())
							if (depth >= MAX_PORTAL_DEPTH)
								continue;
							for (Portal* po : p->cbsPortals)
								h.pq.push_back(po);
						}

						if (done)
							break;
					}
				}
				hv[0].portal->stencilVal = 1;
				size_t count = 1;
				for (size_t i = 1; i < hv.size(); ++i) {
					count += (uint8_t)hv[i - 1].count;
					hv[i].portal->stencilVal = count;
				}
			}
			
			

			// draw portal(s)
			{
				size_t renderDepth = 0;

				// fill portalsToRender queue with initial portals
				list<Portal*> portalsToRender;
				portalsToRender.push_front(&p1);
				portalsToRender.push_front(&p2);
				portalsToRender.push_front(&p3);
				portalsToRender.push_front(&p4);
				portalsToRender.push_front(&p5);
				//portalsToRender.push_front(&p6);
				portalsToRender.push_front(&p7);
				//portalsToRender.push_front(&p8);
				unordered_map<const Portal*, glm::mat4> wtvs;

				// set prevStencil  values to zero for each portal
				for (Portal* p : portalsToRender) {
					p->prevStencil = 0;
					wtvs[p] = camera.GetWorldToViewMatrix();
				}


				// set some shader values that don't change during the frame
				Shader* sp = bulbSP;
				sp->setMat4("projection", projection);
				glClear(GL_STENCIL_BUFFER_BIT);
				glEnable(GL_STENCIL_TEST);
				while (!portalsToRender.empty()) {

					// render each portal plane with clear color, setting stencil buffer value to portal id
					UpdateStencil(portalsToRender, wtvs);

					glClear(GL_DEPTH_BUFFER_BIT);

					// draw portals
					for (const Portal* p : portalsToRender) {
						wtvs[p] = DrawPortal(*p, *p->dest, wtvs[p]);
					}

					for (Portal* p : portalsToRender) {
						p->prevStencil = p->stencilVal;
					}

					++renderDepth;

					size_t len = portalsToRender.size();
					for (size_t i = 0; i < len; ++i) {
						Portal* p = portalsToRender.front();
						portalsToRender.pop_front();
						//if (renderDepth < p->GetMaxRenderDepth()) {
						if (renderDepth < MAX_PORTAL_DEPTH) {
							for (Portal* port : p->cbsPortals) {
								port->prevStencil = p->stencilVal;
								port->stencilVal = p->stencilVal;
								portalsToRender.push_back(port);
								wtvs[port] = wtvs[p];
							}
						}
					}
					

				}
				glDisable(GL_STENCIL_TEST);
			}
		}

		
		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	//clean glfw resources
	glfwTerminate();
	return 0;
}

void UpdateStencil(const list<Portal*>& pl, unordered_map<const Portal*, glm::mat4>& wtvs) {
	// first rendering of portals
	if (pl.front()->prevStencil == 0) {
		for (const Portal* p : pl) {
			PrepareDrawPortal(*p, wtvs[p]);
		}
	}
	else {
		map <uint8_t, map<uint8_t, Portal*>> pmap;
		for (Portal* p : pl) {
			if (pmap.find(p->prevStencil) == pmap.end()) {
				pmap.insert({ p->prevStencil, map<uint8_t, Portal*>()});
			}
			pmap[p->prevStencil][p->stencilVal] = p;
		}

		MeshRenderer* r = pl.front()->GetComponent<MeshRenderer>();
		Shader* sp = r->GetMaterial()->GetShader();

		sp->use();
		sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

		for (auto& m : pmap) {
			//glStencilFuncSeparate(GL_FRONT, GL_EQUAL, m.second.begin()->second->prevStencil, 0xFF);
			//glStencilFuncSeparate(GL_BACK, GL_NEVER, m.second.begin()->second->prevStencil, 0xFF);
			glStencilFunc(GL_EQUAL, m.second.begin()->second->prevStencil, 0xFF);
			for (auto& e : m.second) {
				Portal* p = e.second;
				sp->setMat4("objectToWorld", p->transform.GetTransformMatrix());
				sp->setMat4("worldToView", wtvs[p]);
				p->stencilVal += 1;
				p->prevStencil += 1;
				r->Draw();
			}
			m.second.erase(m.second.begin());
		}

	}
}

void PrepareDrawPortal(const Portal& p, const glm::mat4& worldToView) {
	glStencilFunc(GL_ALWAYS, p.stencilVal, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	MeshRenderer* r = p.GetComponent<MeshRenderer>();
	Shader* sp = r->GetMaterial()->GetShader();

	glm::mat4 objectToWorld = p.transform.GetTransformMatrix();
	sp->use();
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("objectToWorld", objectToWorld);
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw();
}

glm::mat4 DrawPortal(const Portal& p1, const Portal& p2, const glm::mat4& worldToView) {

	// update world to view matrix 
	glm::mat4 newWTV = worldToView * p1.GetPortallingMat();			    

	::worldToView = newWTV;

	// draw the scene from a new perspective
	{
		glStencilFunc(GL_EQUAL, p1.stencilVal, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		Shader* sp = crateRenderer->GetMaterial()->GetShader();
		sp->use();
		sp->setVec4("portalPlaneEq", p2.GetViewspacePortalEquation(newWTV));
		

		glEnable(GL_CLIP_DISTANCE0);
		DrawScene();
		glDisable(GL_CLIP_DISTANCE0);
	}

	return newWTV;
}



void DrawScene() {
	Shader* sp = crateRenderer->GetMaterial()->GetShader();
	// set light properties
	//lightColor.x = sin(glfwGetTime() * 2.0f);
	//lightColor.y = sin(glfwGetTime() * 0.7f);
	//lightColor.z = sin(glfwGetTime() * 1.3f);
	
	
	sp->use();

	// draw crates
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	/*for (int i = 0; i < 10; i++) {
		Transform transform;
		transform.rotation.SetEulerAngles(glm::vec3(i* 20.0f, i*45.0f, 0.0f));
		transform.position = cubePositions[i];
		//transform.SetScale(glm::vec3(0.75f, 2.5f, 1.5f));

		glm::mat4 objectToWorld = transform.GetTransformMatrix();
		crateRenderer->GetMaterial()->GetShader()->setMat4("objectToWorld", objectToWorld);
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
		sp->SetMat3("normalMatrix", normalMatrix);
		crateRenderer->Draw();
	}*/

	// draw floor
	/*sp = floorRenderer->GetMaterial()->GetShader();
	sp->use();
	Transform transform;
	transform.position = glm::vec3(0, 0.0f, 0.0f);
	//transform.rotation = glm::vec3(180.0f, 0.0f, 0.0f);
	transform.SetScale(glm::vec3(10));
	glm::mat4 objectToWorld = transform.GetTransformMatrix();
	sp->setMat4("objectToWorld", objectToWorld);
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
	sp->SetMat3("normalMatrix", normalMatrix);
	floorRenderer->Draw();*/

	// draw grass
	/*sp = grassRenderer->GetShader();
	sp->use();
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	for (int i = 0; i < 8; i++) {
		Transform transform;
		transform.rotation.SetEulerAngles(glm::vec3(0.0f, i*17.5f, 0.0f));
		transform.position = grassPositions[i];

		glm::mat4 objectToWorld = transform.GetTransformMatrix();
		sp->setMat4("objectToWorld", objectToWorld);
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
		sp->SetMat3("normalMatrix", normalMatrix);
		grassRenderer->Draw();
	}*/

	// draw bulb
	sp = bulbRenderer->GetMaterial()->GetShader();
	sp->use();
	// set uniform values
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	sp->setVec3("lightColor", lightColor);
	sp->setMat4("objectToWorld", bulbTransform.GetTransformMatrix());
	bulbRenderer->Draw();

	// draw monkey
	sp = monkeyRenderer->GetMaterial()->GetShader();
	sp->use();
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	sp->setVec3("lightColor", lightColor);
	Transform t;
	t.position = { 0.0f, 1.0f, 5.0f };
	t.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.2f);
	sp->setMat4("objectToWorld", t.GetTransformMatrix());
	monkeyRenderer->Draw();

	// draw portal scenes
	sp->setVec3("dirLight.color", lightColor * 0.5f + 0.5f);
	sp->setFloat("dirLight.intensity", 2.0f);
	sp->setVec3("dirLight.direction", glm::mat3(worldToView) * glm::vec3(0.0f, -1.0f, 0.0f));
	sp->setFloat("dirLight.ambientStrength", 0.2f);
	sp->setVec3("pointLight.position", worldToView * glm::vec4(bulbTransform.position, 1.0f));
	sp->setFloat("material.shiness", 32.0f);
	sp = sceneRenderer->GetMaterial()->GetShader();
	sp->use();
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	sp->setVec3("lightColor", lightColor);
	t.position = { 0.0f, 0.0f, 0.0f };
	t.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
	sp->setMat4("objectToWorld", t.GetTransformMatrix());
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * t.GetTransformMatrix())));
	sp->SetMat3("normalMatrix", normalMatrix);
	sceneRenderer->Draw();
	
	sp = sceneRenderer->GetMaterial()->GetShader();
	sp->use();
	sp->setMat4("worldToView", worldToView);
	sp->setMat4("projection", projection);
	sp->setVec3("lightColor", lightColor);
	t.position = { 0.0f, 0.0f, 50.0f };
	t.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
	sp->setMat4("objectToWorld", t.GetTransformMatrix());
	normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * t.GetTransformMatrix())));
	sp->SetMat3("normalMatrix", normalMatrix);
	scene2Renderer->Draw();

	// draw windows
	if (false)
	{
		sp = windowRenderer->GetMaterial()->GetShader();
		sp->use();
		sp->setMat4("worldToView", worldToView);
		sp->setMat4("projection", projection);
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
			sp->setMat4("objectToWorld", objectToWorld);
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldToView * objectToWorld)));
			sp->SetMat3("normalMatrix", normalMatrix);
			windowRenderer->Draw();
		}
	}
}


void OnPlayerTriggerEnter(const RayHit& hit) {

	// a matrix and a quaternion that are needed for translation of position and rotation
	static const glm::mat4 negXZ{-1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, -1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f };
	static const Rotator rot = glm::vec3(0.0f, 180.0f, 0.0f);

	
	Portal* p = dynamic_cast<Portal*>(hit.collider->GetOwner());
	// entered a portal
	if (p != nullptr) {
		Transform t = player.camera->GetTransform();
		Transform p1 = p->transform;
		Transform p2 = p->dest->transform;
		glm::mat4 posMat = p1.GetInverseTransformMatrix();
		posMat = negXZ * posMat;
		posMat = p2.GetTransformMatrix() * posMat;
		t.position = posMat * glm::vec4(t.position, 1.0f);
		t.rotation = p2.rotation.GetQuaterion() * rot.GetQuaterion() * glm::inverse(p1.rotation.GetQuaterion()) * t.rotation.GetQuaterion();
		player.SetCameraTransfrom(t);
	}
}
