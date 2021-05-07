
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
#include "MoverComponent.h"
#include "UniformBufferObject.h"
#include "Lighting.h"
#include "Shadows.h"
#include "Common.h"
#include "PortalSpace.h"

using namespace std;

// constants
const int MAX_PORTAL_DEPTH = 2;
const GLsizei PORTAL_CUBE_MAP_SIZE = 1024;


void PrerenderPortal(const Portal& p, GLuint& outCm);
void OnPlayerTriggerEnter(const RayHit& hit);

Camera camera;
Player player(&camera);
PlayerController pc(&player);
Physics physics;
DirLight dirLight;

Shader* shadowmapShader;
GLuint shadowMap;
GLuint stencil_view;
Shadows* shadows;

Lighting* lighting;

// color values
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 0.5f, 0.31f);

void CalcVisibleDirLights();

void ForwardRenderScene(const Camera& camera);


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
	pc.ProcessInput(window, (float)mouseX - oldMouseX, (float)mouseY - oldMouseY);
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

// Renderers
MeshRenderer* crateRenderer;
MeshRenderer* floorRenderer;
MeshRenderer* grassRenderer;
MeshRenderer* monkeyRenderer;
MeshRenderer* sceneRenderer;
MeshRenderer* scene2Renderer;
MeshRenderer* fsqRenderer;


//
glm::vec2 portalDims = glm::vec2(0.55f, 1.2f);
// Portals
Portal p1;
Portal p2;
Portal p3;
Portal p4;
Portal p5;
Portal p6;
Portal p7;
Portal p8;
PortalRenderTree prTree;

// actors
Actor mnk;
Actor ps1;
Actor ps2;
Actor bulb;
Actor monke;
Actor monke2;

// PortalSpace 2
PortalSpace portalSpace2;

void ClearVisibleDirLights();


int main() {
	string s;
	cin >> s;
	//initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGLProject", NULL, NULL);
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
	unsigned char* data = stbi_load("D:\\MyFiles\\SomeStuff\\container2.jpg", &width, &height, &nrChannels, 0);
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
	data = stbi_load("D:\\MyFiles\\SomeStuff\\container2spec.jpg", &width, &height, &nrChannels, 0);
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
	vector<Vertex> verts(v, v + 36);
	vector<GLuint> inds(indices, indices + 36);
	vector<Texture> texs;
	texs.push_back(diffTex);
	texs.push_back(specTex);
	Material contMat;
	contMat.textures = texs;
	Mesh container = Mesh(verts, inds);

	// create flloor mesh
	verts.clear();
	verts.push_back({ glm::vec3(1, 0, -1), glm::vec3(0,1,0), glm::vec2(1, 1) });
	verts.push_back({ glm::vec3(1, 0, 1), glm::vec3(0,1,0), glm::vec2(1, 0) });
	verts.push_back({ glm::vec3(-1, 0, 1), glm::vec3(0,1,0), glm::vec2(0, 0) });
	verts.push_back({ glm::vec3(-1, 0, -1), glm::vec3(0,1,0), glm::vec2(0, 1) });
	inds.clear();
	inds.push_back(0);
	inds.push_back(1);
	inds.push_back(2);
	inds.push_back(2);
	inds.push_back(3);
	inds.push_back(0);
	Mesh floor = Mesh(verts, inds);

	// create portal plane mesh
	
	//glm::vec2 portalDims = glm::vec2(0.5f, 0.5f);
	verts.clear();
	verts.push_back({ glm::vec3(portalDims.x, -portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 0) });
	verts.push_back({ glm::vec3(portalDims.x, portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 1) });
	verts.push_back({ glm::vec3(-portalDims.x, portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 1) });
	verts.push_back({ glm::vec3(-portalDims.x, -portalDims.y, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 0) });
	texs.clear();
	texs.push_back(scene1Tex);
	sceneMat;
	sceneMat.textures = texs;
	Mesh portalPlane = Mesh(verts, inds);


	// create fullscreen quad mesh
	verts.clear();
	verts.push_back({ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 0) });
	verts.push_back({ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0,0,-1), glm::vec2(1, 1) });
	verts.push_back({ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 1) });
	verts.push_back({ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0,0,-1), glm::vec2(0, 0) });
	Mesh fsq = Mesh(verts, inds);

	// create some global objects
	CreateGlobalMatricesBuffer();
	CreatePortalBlockUbo();
	lighting = new Lighting();
	shadows = new Shadows();

	ShaderCompiler comp;
	//create shaderProgram
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* sp = comp.Compile();
	sp->BindUBO(globalMatrices);
	lighting->BindUboToShader(sp);
	sp->BindUBO(GetPortalBlockUbo());

	// create bulb shader program
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbFragmentShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbVertexShader.vs");
	Shader* bulbSP = comp.Compile();
	bulbSP->BindUBO(globalMatrices);
	bulbSP->BindUBO(GetPortalBlockUbo());

	// create grass shader program
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\GrassFragmentShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* grassShader = comp.Compile();
	grassShader->BindUBO(globalMatrices);
	grassShader->BindUBO(GetPortalBlockUbo());

	// create shadowmap shader
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\smVertex.vs");
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\smFragment.fsf");
	shadowmapShader = comp.Compile();
	shadowmapShader->BindUBO(globalMatrices);
	shadowmapShader->BindUBO(GetPortalBlockUbo());

	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\fsqVertex.vs");
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\fsqFragment.fsf");
	Shader* fsqShader = comp.Compile();

	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\ppVertex.vs");
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\ppFragment.fsf");
	Shader* ppShader = comp.Compile();
	ppShader->BindUBO(globalMatrices);

	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\smVertex.vs");
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\clearDepthFragment.fsf");
	Shader* clearDepthShader = comp.Compile();
	clearDepthShader->BindUBO(globalMatrices);
	clearDepthShader->BindUBO(GetPortalBlockUbo());

	contMat.shader = sp;
	sceneMat.shader = grassShader;

	clearDepthMat = new Material(clearDepthShader);


	MeshLoader ml;
	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\monkey.fbx");
	Mesh monkey = ml.GetMesh(0);

	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\portal_scene.fbx");
	Mesh portalScene = ml.GetMesh(1);

	ml.OpenFile("D:\\MyFiles\\ITMO\\Year4\\computerGraphics\\portal_scene2_later.fbx");
	Mesh portalScene2 = ml.GetMesh(0);

	sceneMat.shader = sp;


	// Create renderers
	crateRenderer = new MeshRenderer(&container, &contMat);
	floorRenderer = new MeshRenderer(&floor, &contMat);
	grassRenderer = new MeshRenderer(&portalPlane, &sceneMat);

	monkeyRenderer = new MeshRenderer(&monkey, &contMat);
	sceneRenderer = new MeshRenderer(&portalScene, &sceneMat);
	scene2Renderer = new MeshRenderer(&portalScene2, &sceneMat);
	fsqRenderer = new MeshRenderer(&fsq, new Material(fsqShader));
	MeshRenderer* ppRenderer = new MeshRenderer(&portalPlane, new Material(ppShader));

	smMat = new Material(shadowmapShader);

	ps1.AddComponent(sceneRenderer);
	ps1.transform.position = { 0.0f, 0.0f, 0.0f };
	ps1.transform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
	ps2.AddComponent(scene2Renderer);
	ps2.transform.position = { 0.0f, 0.0f, 50.0f };
	ps2.transform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);

	// confugure bulb
	bulb.transform.position = glm::vec3(0.0f, 1.5f, 41.0f);
	bulb.transform.rotation.SetEulerAngles(glm::vec3(0.0f, 0.0f, 45.0f));
	bulb.transform.SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
	bulb.AddComponent(new MeshRenderer(&container, new Material(bulbSP)));


	// Moving logic
	mnk.AddComponent(monkeyRenderer);
	MoverComponent* mc = new MoverComponent();
	mc->startPos = glm::vec3(3.5f, 1.7f, -5.2f);
	mc->endPos = glm::vec3(3.5f, 1.7f, 5.2f);
	mnk.AddComponent(mc);


	

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
		p->AddComponent(new MeshRenderer(&portalPlane, new Material(bulbSP)));
	}

	// Create portals
	{

		// portal 1
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, 54.15f };
			t.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			p1.transform = t;
			p1.stencilVal = 1;
			p1.dest = &p4;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p1.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p1.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 2
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, 35.5f };
			t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
			p2.transform = t;
			p2.stencilVal = 2;
			p2.dest = &p3;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p2.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p2.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 3
		{
			Transform t;
			t.position = glm::vec3(0.0f, 1.25f, 2.1f);
			//t.position = glm::vec3(5.0f, 1.25f, 2.1f);
			t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
			p3.transform = t;
			p3.stencilVal = 3;
			p3.dest = &p2;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p3.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p3.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 4
		{
			Transform t;
			t.position = { 0.0f, portalDims.y, -2.05f };
			t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
			p4.transform = t;
			p4.stencilVal = 4;
			p4.dest = &p1;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p4.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p4.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 5
		{
			Transform t;
			t.position = { 2.1f, portalDims.y, 0.0f };
			//t.position = { 2.f, portalDims.y, 6.3f };
			t.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
			//t.position = { 1.5f, portalDims.y, 3.0f };
			//t.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			p5.transform = t;
			p5.stencilVal = 1;
			p5.dest = &p8;
			//p5.cbsPortals.push_back(&p8);
			//p5.cbsPortals.push_back(&p1);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p5.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p5.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 6
		{
			Transform t;
			//t.position = { 15.0f, portalDims.y, 50.0f };
			t.position = { 1.0f, portalDims.y, 50.0f };
			t.rotation = glm::vec3(0.0f, -90.0f, 0.0f);
			p6.transform = t;
			p6.stencilVal = 1;
			p6.dest = &p7;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p6.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p6.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 7
		{
			Transform t;
			//t.position = { -2.1f , portalDims.y, .0f };
			t.position = { 0, portalDims.y, 6.3f };
			t.rotation = glm::vec3(0.0f, -90.0f, 0.0f);
			p7.transform = t;
			p7.stencilVal = 1;
			p7.dest = &p6;
			//p7.cbsPortals.push_back(&p6);
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p7.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p7.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}

		// portal 8
		{
			Transform t;
			t.position = { -4.242f, portalDims.y, 50.0f };
			t.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
			p8.transform = t;
			p8.stencilVal = 1;
			p8.dest = &p5;
			PlaneCollider* col = new PlaneCollider();
			col->halfDims = portalDims * 1.5f;
			col->isTrigger = true;
			p8.AddComponent(col);
			Physics::Instance()->AddCollider(col);
			p8.plane = RectanglePlane(t.position, t.rotation.GetForwardVector(), portalDims);
		}
	}


	PortalSpace* dps = GetDefaultPortalSpace();
	dps->AddActor(&mnk);
	dps->AddActor(&ps1);
	dps->AddPortal(&p3);
	dps->AddPortal(&p4);
	dps->AddPortal(&p5);
	dps->AddPortal(&p7);
	dps->dirLights.insert(&dirLight);
	dirLight.portalSpace = dps;
	portalSpace2.AddActor(&ps2);
	portalSpace2.AddActor(&bulb);
	portalSpace2.AddPortal(&p1);
	portalSpace2.AddPortal(&p2);
	portalSpace2.AddPortal(&p6);
	portalSpace2.AddPortal(&p8);

	currentPortalSpace = dps;

	// object bewtween portals
	{
		monke.AddComponent(new MeshRenderer(&monkey, &contMat));
		Transform t;
		t.position = { 0.1f, portalDims.y, 6.2f };
		t.SetScale(glm::vec3(1.0f, 1.0f, 1.0f) / 3.0f);
		monke.transform = t;
		monke2.AddComponent(new MeshRenderer(&monkey, &contMat));
		monke2.transform = p7.PortalTransformToDest(t);
	}
	// inbetween objects
	{
		InBetweenObject* o = new InBetweenObject();
		o->actor = &monke;
		o->enteredNormal = -p7.transform.rotation.GetForwardVector();
		o->enteredPortal = &p7;
		dps->inbetweenObjects.insert(o);
		o = new InBetweenObject();

		o->actor = &monke2;
		o->enteredNormal = -p7.dest->transform.rotation.GetForwardVector();
		o->enteredPortal = p7.dest;
		p7.dest->GetPortalSpace()->inbetweenObjects.insert(o);
	}


	// initialize mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	oldMouseX = (float)mouseX;
	oldMouseY = (float)mouseY;



	// set initial camera position
	{
		Transform t;// = camera.GetTransform();
		t.position = glm::vec3(0.0f, 1.25, -2.05f);
		t.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
		camera.SetTransform(t);
	}

	// 
	player.onTriggerHitCallback = OnPlayerTriggerEnter;


	// configure dir light
	{
		dirLight.transform.position = glm::vec3(6.0f, 8.2f, 6.0f);
		//dirLight.transform.position = glm::vec3(6.0f, 8.2f, 6.0f) -
		//	dirLight.transform.rotation.GetForwardVector() * 11.0f;
		dirLight.transform.rotation = glm::vec3(-126.295, -41.2203, 180);
		dirLight.intensity = 2.0f;
		dirLight.ambientStrenght = 0.2f;
		//dirLight.farPlane = 200.0f;
	}

	// testing 
	//GLuint cm;
	//PrerenderPortal(p4, cm);
	// end testing



	// 
	CalcVisibleDirLights();

	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		mc->Update();

		//input
		processInput(window, sp);

		//rendering
		if (false)
		{
			//open gl render config
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// temp 
			//{
			//	Transform t = p7.transform;
			//	static glm::vec3 rot = glm::vec3(0.0f, -10.0f, 0.0f);;
			//	//rot.y += 0.5f;
			//	t.rotation = rot;
			//	t.position = { -2.1f , portalDims.y, 5.0f };
			//	p7.transform = t;
			//	t = p5.transform;
			//	t.position = glm::vec3(2.1f, portalDims.y, -5.0f);
			//	p5.transform = t;
			//	ClearVisibleDirLights();
			//	CalcVisibleDirLights();
			//}

			// render shadowmaps
			{
				if (PortalSpace::shadowmappedLights.size() > 0)
					shadows->RenderShadowmap(*(*dps->shadowmappedLights.begin()));
			}



			// draw the scene the first time
			if (false)
			{
				currentPortalSpace->Draw(&camera);
			}

			glm::vec3 visibleNormal = p7.transform.rotation.GetForwardVector();
			// temp draw the current space slice
			if (false)
			{
				
				glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
				glm::vec3 vn = normalMat * visibleNormal;
				glm::vec4 eq = p7.GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
				if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
					eq = -eq;
				SetGlobalViewspacePortalEquation(eq);
				glEnable(GL_CLIP_DISTANCE0);
				monke.GetComponent<MeshRenderer>()->Draw();
				glDisable(GL_CLIP_DISTANCE0);
			}

			// draw shadowmap on fsq
			if (false)
			{
				glDisable(GL_DEPTH_TEST);
				glBindTexture(GL_TEXTURE_2D, shadowMap);
				fsqRenderer->GetMaterial()->shader->Use();
				fsqRenderer->GetMaterial()->shader->setInt("screenTex", 0);
				fsqRenderer->Draw();
				glEnable(GL_DEPTH_TEST);
			}

			// draw prerendered portal
			if (false)
			{
				Transform t;
				t.position = { 1.0f, portalDims.y, -2.2f };
				//t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
				Shader* s = ppRenderer->GetMaterial()->GetShader();
				glm::mat4 scale = glm::mat4(1.0f);
				scale[0][0] = 0.5f / portalDims.x;
				scale[1][1] = 0.5f / portalDims.y;
				glm::mat4 scale2 = glm::mat4(1.0f);
				//scale2[0][0] = 8.0f/6.0f;
				//scale2[1][1] = 0.5f / portalDims.y;
				s->Use();
				glm::vec3 psCamPos = scale2 * scale * t.GetInverseTransformMatrix() * glm::vec4(camera.GetTransform().position, 1.0f);
				s->setVec3("psCamPos", psCamPos);
				// do we need this here?
				{
					SetGlobalViewMatrix(camera.GetWorldToViewMatrix());
					SetGlobalProjectionMatrix(camera.GetProjectionMatrix());
				}
				//s->setMat4("objectToWorld", t.GetTransformMatrix());
				s->setMat4("portalDimsScaler", scale);
				ppRenderer->Draw();
			}

			// draw portals - new algo
			//if (false)
			{
				// algorithm(using PortalRenderTree):
				// having rendered the scene
				// get portals at depth 1
				// render portal planes setting the stencil values if the depth test passes
				// clear depth where the portals' respective stencil values are set (is one fullscreen render call better than a draw call for each plane? - it can work only for depth 1)
				// render contents of each of the portals
				// depth 2+:
				// get portals at the depth
				// render portal planes setting the stencil values:
				//	at depth 2+ rendering portals is harder since i can only increment stencil values (I can download AMD_stencil_operation_extended extension for amd cards to change it to any value)
				//	without extension:
				//		the stencil values have to be updated incrementally
				//		variation 1:
				//		render all portal planes each time incrementing the stencil value until the stencil value is what it's supposed to be for the portal
				//		(remove the portal from the list when the value is right, keep rendering until the list is empty)
				//		if portals are grouped by curent stencil value (parent portal's value at firts iteration) gpu instancing can be used to make rendering faster (if the portals use the same mesh)
				//		variation 2:
				//		render the portal planes without modifying the stencil (so that stencil does not interfere with depth oreder of portals)
				//		since we render the portal planes anyway we can draw them before rendering the contents of the parent portal(s) (which is an optimization - a selective z-prepass) - wrong 'cause it obstructs scene objects
				//		set depth test to equal
				//		set stencil values for each portal individually (incrementally obviously)
				//		slightly increases number of draw calls (compared to variation 1)
				//		removes some of the overhead of variation 1 and makes the algo simpler
				//	with extension:
				//		set stencil test to equal to parent's
				//		draw portal planes (obviously in parent's stencil) without modifying stencil
				//		set depth test to equal
				//		draw the portal planes  setting the new stencil values
				//		return depth test to previous value
				//	render the portal's contents
				// keep rendering until there's no more portals left
				// 
				//

				// compute the portal rendering tree
				prTree.ConstructTree(currentPortalSpace->GetPortals(), camera);

				// clear stencil
				glClear(GL_STENCIL_BUFFER_BIT);
				glEnable(GL_STENCIL_TEST);

				// for all possible depths
				size_t depth = 0;
				for (auto iter = prTree.Begin(); iter != prTree.End(); depth++)
				{

					// render the portal planes inside the portals without changing the stencil values
					auto it = iter;
					{
						
						// get iterator to next depth
						while (it != prTree.End() && (*it)->GetDepth() == depth)
						{
							++it;
						}
						auto i = it;
						glEnable(GL_DEPTH_CLAMP);
						while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
						{
							BeginDrawInsidePortal(*(*i)->GetParent());
							DrawPortalPlane(*(i), false);
							EndDrawInsidePortal();
							++i;
						}
						glDisable(GL_DEPTH_CLAMP);

					}


					// Render portals' contents
					{
						while (iter != prTree.End() && (*iter)->GetDepth() == depth)
						{
							DrawPortalContents(**iter);
							++iter;
						}
					}


					// update stencil
					{
						GLint dFunc;
						glGetIntegerv(GL_DEPTH_FUNC, &dFunc);
						glDepthFunc(GL_EQUAL);
						if (depth == 0)
						{
							auto i = it;
							{
								glEnable(GL_DEPTH_CLAMP);
								while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
								{
									DrawPortalPlane(*(*i), true);
									++i;
								}
								glDisable(GL_DEPTH_CLAMP);
							}
						}
						else
						{

							auto i = it;
							while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
							{
								BeginDrawInsidePortal(*(*i)->GetParent());
								glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
								stencil_t s = (*i)->GetParent()->GetStencil();
								while (s < (*i)->GetStencil())
								{
									glStencilFunc(GL_EQUAL, ++s, 0xFF);
									DrawPortalPlane(**i);
								}
								EndDrawInsidePortal();
								++i;
							}
						}
						glDepthFunc(dFunc);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					}

					// clear depth - right now i don't need depth buffer, so i can just clear it
					// alternatively i can use glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE), glDepthFunc(GL_ALWAYS) and a shader that clears the depth
					// for each drawn plane (or a fullscreen plane with glStencilFunc(GL_GREATER, 0, 0xFF), glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP) - possible only the first time)
					glClear(GL_DEPTH_BUFFER_BIT);



					// temp draw the inside slice
					if (depth == 1 && false)
					{
						auto i = iter;
						while (i != prTree.End() && (*i)->GetDepth() == depth)
						{
							if ((*i)->GetPortal() == &p7)
							{
								SetGlobalViewMatrix(camera.GetWorldToViewMatrix());
								SetGlobalProjectionMatrix(camera.GetProjectionMatrix());
								glStencilFunc(GL_EQUAL, (*i)->GetStencil(), 0xFF);
								glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
								glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
								glm::vec3 vn = normalMat * visibleNormal;
								glm::vec4 eq = p7.GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
								if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
									monke.GetComponent<MeshRenderer>()->Draw();
								//EndDrawInsidePortal();
								break;
							}
							++i;
						}
					}


				}

				glDisable(GL_STENCIL_TEST);
			}
			shadows->FreePool();
		}
		
		
		ForwardRenderScene(camera);

		//check all the events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	//clean glfw resources
	glfwTerminate();
	return 0;
}




void OnPlayerTriggerEnter(const RayHit& hit) {

	// a matrix and a quaternion that are needed for translation of position and rotation
	static const glm::mat4 negXZ{ -1.0f, 0.0f, 0.0f, 0.0f,
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
		currentPortalSpace = p->dest->GetPortalSpace();
	}
}

void PrerenderPortal(const Portal& p, GLuint& outCm) {
	// create cubemap texture to write to
	glGenTextures(1, &outCm);
	glBindTexture(GL_TEXTURE_CUBE_MAP, outCm);
	for (GLuint i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, PORTAL_CUBE_MAP_SIZE,
			PORTAL_CUBE_MAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// create depth/stencil renderbuffer
	GLuint ds;
	glGenRenderbuffers(1, &ds);
	glBindRenderbuffer(GL_RENDERBUFFER, ds);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, PORTAL_CUBE_MAP_SIZE, PORTAL_CUBE_MAP_SIZE);

	// create new framebuffer
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ds);

	// create world to view matrices for each face
	vector<glm::mat4> wtvs;
	Transform t;
	// positive x face
	t = p.dest->transform;
	t.rotation.RotateArounAxis(-90.0f, t.rotation.GetUpVector());
	wtvs.push_back(t.GetInverseTransformMatrix());
	// negative x face
	t = p.dest->transform;
	t.rotation.RotateArounAxis(90.0f, t.rotation.GetUpVector());
	wtvs.push_back(t.GetInverseTransformMatrix());
	// positive y face
	t = p.dest->transform;
	//t.rotation.RotateArounAxis(-90.0f, t.rotation.GetRightVector());
	wtvs.push_back(t.GetInverseTransformMatrix());
	// negative y face
	t = p.dest->transform;
	//t.rotation.RotateArounAxis(270.0f, t.rotation.GetRightVector());
	wtvs.push_back(t.GetInverseTransformMatrix());
	// positive z face
	t = p.dest->transform;
	t.rotation.RotateArounAxis(180.0f, t.rotation.GetUpVector());
	wtvs.push_back(t.GetInverseTransformMatrix());
	// negative z face
	t = p.dest->transform;
	wtvs.push_back(t.GetInverseTransformMatrix());


	// draw the scene for each face
	//glCullFace(GL_BACK);
	//glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, PORTAL_CUBE_MAP_SIZE, PORTAL_CUBE_MAP_SIZE);
	Camera cam;
	cam.SetProjectionMatrixPerspective(glm::radians(90.0f), 1.0f, 0.01f, 100.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	for (GLsizei i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, outCm, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		cam.SetWorldToViewMatrix(wtvs[i]);
		currentPortalSpace->Draw(&cam);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}

void CalcVisibleDirLights() {
	std::vector<PortalSpace*> spaces;
	spaces.push_back(GetDefaultPortalSpace());
	spaces.push_back(&portalSpace2);
	// first insert lights that exist in a space itself
	for (PortalSpace* ps : spaces)
	{
		for (DirLight* l : ps->dirLights)
		{
			if (ps->shadowmappedLights.size() < Lighting::MAX_DIR_LIGHT_COUNT)
			{
				PortalShadowedDirLight* sdl = new PortalShadowedDirLight();
				sdl->light = l;
				PerPortalDirLightData d;
				d.direction = l->transform.rotation.GetForwardVector();
				d.lightSpaceMatrix = l->GetLightSpaceMatrix();
				d.stencilVal = 0;
				d.lsPortalEq = glm::vec4(0.0f);
				sdl->perPortal.insert({ nullptr, d });
				PortalSpace::shadowmappedLights.insert(sdl);
				ps->drawableDirLights.insert(new DrawableDirLight({ sdl, nullptr }));
			}
		}
	}

	// then insert lights incoming from neighbor spaces
	for (PortalSpace* ps : spaces)
	{
		for (Portal* p : ps->GetPortals())
		{
			PortalSpace* other = p->dest->GetPortalSpace();
			for (PortalShadowedDirLight* l : PortalSpace::shadowmappedLights)
			{
				// must use only the other portalSpace's own lights
				if (l->light->GetPortalSpace() != other)
					continue;
				if (ps->drawableDirLights.size() >= Lighting::MAX_DIR_LIGHT_COUNT)
					break;

				ps->shadowmappedLights.insert(l);
				PerPortalDirLightData d;
				// TODO: calculate direction properly
				// since portallingMat doesn't use scale
				// we can use upper right corner of the portalling matrix 
				// to modify the direction
				glm::mat4 pmat = p->dest->GetPortallingMat();
				d.direction = glm::mat3(pmat) * l->light->transform.rotation.GetForwardVector();
				d.lightSpaceMatrix = l->light->GetLightSpaceMatrix() * pmat;
				d.stencilVal = (uint32_t)l->perPortal.size();
				d.lsPortalEq = p->GetViewspacePortalEquation(d.lightSpaceMatrix, true);
				l->perPortal.insert({ p->dest, d });

				ps->drawableDirLights.insert(new DrawableDirLight({ l, p->dest }));
			}
			if (ps->drawableDirLights.size() >= Lighting::MAX_DIR_LIGHT_COUNT)
				break;
		}
	}
}

void ClearVisibleDirLights()
{
	std::vector<PortalSpace*> spaces;
	spaces.push_back(GetDefaultPortalSpace());
	spaces.push_back(&portalSpace2);

	for (auto* space : spaces)
	{
		for (auto* ddl : space->drawableDirLights)
			delete ddl;
		space->drawableDirLights.clear();
	}

	for (auto* sl : PortalSpace::shadowmappedLights)
		delete sl;
	PortalSpace::shadowmappedLights.clear();
}


// what i need:
// generate shadowmaps before adding the lights to lighting
//		becasue the shadowmaps are reused
//		
// save the light/portal unique stencil values
// and the matrices
// To generate a light's shadowmap i need:
//		a list of portals the light might interact with
//		(a list og portals in the same Portal Space where the light is)
//
//	To generate shadowmap for all lights i need:
//		a list of lights each of which has a list of portals
// 
// the light should probably store the info of which PortalSpace it belongs to
// as should all actors
// 
// RenderShadowmap needs to store the PerPortalLightData
// PortalShadowedDirLight 
// 
// Shadows.cpp/Shadows.h need to have a pool of textures for the shadowmaps
// only 4(depends on the number of lights) of them are used at the same time, anyway
// 
// used lights need to be removed after each PortalArea rendering
// 
// TODO:
// update actor to include reference to portalSpace +
// add default portal space +
// add actors to portal space(s) +
// draw portal space instead of draw scene
// update shadowmap rendering
//		shadowmap pool
// update rendering to properly use new shadowmaps and lights
// write down the shadowmap generation algorithm
// replace maps with unordered_maps where appropriate
// calculate PerLightData light direction
// add lightspace portal equation
// free shadowmaps to the pool
// 
// 
// 
// TODO:
//		keep track of the current portalSpace and change it when entering a portal +
//		update  CalcVisibleDirLights() as it doesn't create a comfortable data structure - i cannot get the lights that are in the space easily +
//		bugfix shadowmap generation as it doesn't seem to include all the portals +
//		select lights based on the currently rendered PortalSpace +
//		update everyplace to use PortalSpace::Draw() instead of DrawScene +
//		update shader to iterate through lights +
//		fix a bug where shadowmap sometimes has a huge offset - it's probably just that dirLights[0] is a different light (cause i use a hashset) +
//		add lightspace portalspace equation to sample shadowmaps properly - it's calculated the same way as viewspace plane equation (for ortho projection) +
//		ambient lighting should only come from the PortalSpace's own lights +
//		calculate light matrices to fit frustum (especially when inside portals)
// 
// 
// portal should be drawn if:
// at least one point's z is greater than zero and  range of dot products with forwarnd vector intersects with range of frustum dot products in either ZOX or YOZ planes
// algo:
//	the 4 points of the portal basically difine a portal's frustum
//  get unit vectors from this values
//	the camera frustum and the "portal's frustum" will intersect if one of either of the frustum's vectors(corner points) is inside the other frustum
//	a point is inside a frustrum if it's between all of it's planes
//	i can get normals of the planes with cross product
//	with normals i can get distance to the planes ((p-q) dot n == p dot n, when q is zero vector) - if all distances are negative (depends on the normal direction) then the point is inside the frustum
//	don't use near or far planes!
//	if they intersect then we draw the portal
//	camera's frustum plane's normals can be precomputed
//	don't draw if all z's are greater than far plane
//	don' draw if every portal's point's length is greater than the lenght of the longet line of the camera's frustum ?
// also need to check visibility through a parent portal
// 
// calculating a portal's bounding frustum and render to texture method can be used to create false recursion layers
//		


// TODO:
// calculate if a point is in a pyramid +
// check if a plane and an infinite Pyramid overlap +
// check plane and pyramid overlap +/-
// Update Camera to allow setting of worldToViewMatrix +
// Update Camera to allow setting of projectionMatrix
// Update Camera to save a Pyramid
// Replace all instances of Cam with Camera
// 
// 
// converting portal plane's points to viewspace:
// pros:
//	camera needs to update its pyramid only when projection parameters change
// cons:
//	every point of a portal plane needs to converted to viewspace
//  every precomputed cross product and every normal also needs to be converted to viewspace or reconstructed in viewspace
// total operations:
// 
// updating camera's pyramid when camera's position changes:
//	pros:
//		portal's fourpoint planes don't need to be converted to viewspace
//	cons:
//		need to minorly update the pyramid class
//		pyramid updates at every position change and every projection parameter change
//	updating a pyramid is reconstructing it
// 
// operations per portal:
//	1 array copy
//	4 vector subtractions
//	4 cross products
// 
// to construct a pyramid I need apex and the 4 base points
// 
// to get the 4 base points:
//	calculate the points in viewspace and convert them to world space
// apex is the camera's position
//		
// TODO: 
//	change FourPointPlane to RectanglePlane and use extent and origin and normal to contruct it
// 
// 
// new algo to check portal visibilty
//  transform points to viewspace
//  check if at least one point is in front of the parent portal plane and z=0 plane
//	project portal's points onto the parent plane and check if they overlap
//  transform points to ndc
//	clamp z to (-1.0; +infinity)
//	check overlap as a box and a plane ((minX;maxX) ovelaps (-1;1) and (minY;maxY) overlaps(-1;1)and (minZ;maxZ) overlaps(-1;1))
//  
// 
// Ovelap 2d
//  takes 2  lenght 4 arrays of 2-dimensional points
//	2 2d boxes (a and b) overlap if either:
//		corner of a is inside b
//		corner of b is inside a
//		edge of a intesects with an adge of b
//			(edge AB intescts edge CD if C and D are on diferent sides of AB and A and B are on different sides of CD)
// 
// 
// a plane and a pyramid intersect if either:
//	a cornder of the plane is inside the pyramid
//	one of the pyramid's edges intersects the plane
//	one of the plane's edges intersects one of the pyramid's faces
//	
// and edge AB intersects face c if 
//	A is on one side of c while B is on the other
//	and 
//  intersection point is within the plane's borders (either rectangle or a triangle)
// 
// 
// TODO:
// rename plane to rectangle +
// rename pyramid to RegularPyramid
// construct RegularPyramid from camera parameters
// construct Ray from 2 points
// generate portal's Rectangle plane using portal's transform not just the normal
// 
// 
// 
// Draw an object that is going throug a portal:
//	I need to know from which direction the opbject is crossing the portal
//	i should know the extent of the object to know when it's done crossing the portal
//	need to use portal plane to cull the part of the pbject that's supposed to be inside
//	if the portal leads to the same space the object needs to be drawn twice in the space
//	
// the algo:
//	check if there're any objects in between portals
//	draw the current space slice - draw the object for current stencil using portal plane as a culling plane - discard the part of the object behind the portal
//	draw the scene
//	draw the portals' planes with stencil
//	clear depth where the portals are
//	draw the portals' contents
//	Draw the "inside space" slice - draw the object for portal's stencil if the the object is behind the portal
//		to know where the object is the object should remember which normal of the portal it's going in
//		also i need update the lightspace matrices for the object
//	we draw the object twice here, so it should be drawn in the same space (since the calculations are not percise, but give the same results for the same input)
//
// complex meshes need to be sliced with portal's rectangle 
// 
// 
// TODO:
// need to unify codebase - use the same algo for rendering shadowmaps as for rendering the scene:
//	add a maxDepth parameter for PortalRenderTree::ConstructTree +
//	precompute viewspace portal equtions - calculate them using the parent portal camera +
//	precompute cameras instead of portalling mats +
//  modify the algo:
//		check visibility based on whether camera uses perspective or orhographic projection
//		put rendering of the current portal space into the algo +
//		insert midportal object rendering into the algo
//		put the algo in a separate fucntion +
// 
// 
// 
// apparently, i accidentally achieved the following:
//	stencil for inside portals is set with glStencilFunc(GL_ALWAYS, ...)
//	it works fine because portal rectangles are prerendered with glDepthFunc(GL_EQUAL) inside the parent portal
//	chances of it working wrong are pretty low (zero when fully clearing depth)
//	this can be used as an optimization
// 
// 
// 
// 
// TODO:
// class for objects that can be mid portal
//	it needs to store normal and portal that it is currently in between
//	for simplicity save an array of such objects in portal space
//	one inbetweenObject for each piece
// get objects that are inbetween portals for a portal space
// draw the current space pieces of the objects before drawing the outer contents
// draw the inside space pieces of the objects after updating stencil (need to update lightspace matrices for proper lighting)
// inside space pieces need to be drawn in parent camera space (which should be the same as child but floating point operations are no to precise)
// so when rendering i must only render the pieces whose entered portal's destination is not the current parent portal
// 
// TODO:
//	add second portal equation
//  Add inbetween object to shadowmapping
//

