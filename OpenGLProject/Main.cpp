
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

using namespace std;

// constants
const int MAX_PORTAL_DEPTH = 2;
const GLsizei SHADOW_MAP_SIZE = 1024 *2;
const GLsizei PORTAL_CUBE_MAP_SIZE = 1024;
const size_t MAX_DIR_LIGHT_COUNT = 4;

struct DirLight {
	Transform transform;
	float intensity = 1.0f;
	float ambientStrenght = 0.1f;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	GLuint smDepth = 0;
	GLuint smStencil = 0;
	glm::mat4 lightSpaceMatrix; // matrix that transforms worldspace coordinates to lightspace

	// shadow rendering parameters
	float nearPlane = 1.0f;
	float farPlane = 20.0f;
	// height and width of the rendering frustrum
	float extent = 20.0f;
};


struct PortalSpace {
	vector<Portal*> portals;
	vector<DirLight> dirLights;
};

size_t numDirLights = 0;
DirLight dirLights[MAX_DIR_LIGHT_COUNT];

UniformBufferObject* globalMatrices;

void CreateGlobalMatricesBuffer();

void SetGlobalViewMatrix(const glm::mat4& view);

void SetGlobalProjectionMatrix(const glm::mat4& projection);

void ApplyLightValuesToShader(Shader* shader, size_t lightIndex);

/*
* needs to take into account all the lights that exist in the space
* and all the lights that are coming into the space
*/
void CalculateVisibleDirLights(PortalSpace* space);


struct Cam {
	glm::mat4 worldToView;
	glm::mat4 projection;
};

// if matOverride is not nullptr DrawScene doesn't change shader that is used
// nor does it set any shader parameters except objectToWorld and cam matrices
void DrawScene(const Cam& cam, Material* matOverride = nullptr);
/*@pre Stencil testing must be enabled
  @pre Stencil buffer should be cleared with 0's
 */
void DrawPortalPlane(const Portal& p, const glm::mat4& worldToView);
glm::mat4 DrawPortal(const Portal& p1, const Cam& cam, Material* matOverride = nullptr);
void PrerenderPortal(const Portal& p, GLuint& outCm);

void ConfigureFBOAndTextureForShadowmap(GLuint& fbo, GLuint& tex, GLuint& stencil_view);
void RenderShadowmap(GLuint fbo, DirLight& light, list<Portal*> portalsToRender);
glm::mat4 GetLightSpaceMatrix(const DirLight& light);
Cam GetLightCam(const DirLight& light);

void UpdateStencil(const list<Portal*>& pl, unordered_map<const Portal*, glm::mat4>& wtvs);

void OnPlayerTriggerEnter(const RayHit& hit);

Camera camera;
Player player(&camera);
PlayerController pc(&player);
Physics physics;
DirLight light;

Shader* shadowmapShader;
GLuint shadowMap;
GLuint stencil_view;

// color values
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

// Renderers
MeshRenderer* crateRenderer;
MeshRenderer* floorRenderer;
MeshRenderer* grassRenderer;
MeshRenderer* monkeyRenderer;
MeshRenderer* sceneRenderer;
MeshRenderer* scene2Renderer;
MeshRenderer* fsqRenderer;

// Materials
Material* smMat;
Material* clearDepthMat;
Material sceneMat;

// Portals
Portal p1;
Portal p2;
Portal p3;
Portal p4;
Portal p5;
Portal p6;
Portal p7;
Portal p8;

size_t renderDepth;

// actors
Actor mnk;
Actor ps1;
Actor ps2;
Actor bulb;

// temp
glm::mat4 portallingMat;


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

	// create GlobalMatrices Unniform Buffer Object
	CreateGlobalMatricesBuffer();

	ShaderCompiler comp;
	//create shaderProgram
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\FragmenShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* sp = comp.Compile();
	sp->BindUBO(globalMatrices);

	// create bulb shader program
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\BulbFragmentShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* bulbSP = comp.Compile();
	bulbSP->BindUBO(globalMatrices);

	// create grass shader program
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\GrassFragmentShader.fsf");
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\VertexShader.vs");
	Shader* grassShader = comp.Compile();
	grassShader->BindUBO(globalMatrices);

	// create shadowmap shader
	comp.SetVertexShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\smVertex.vs");
	comp.SetFragmentShader("D:\\VSProjects\\OpenGLProject\\OpenGLProject\\smFragment.fsf");
	shadowmapShader = comp.Compile();
	shadowmapShader->BindUBO(globalMatrices);

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

	contMat.shader = sp;
	sceneMat.shader = grassShader;

	clearDepthMat = new Material(clearDepthShader);


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
			//t.position = glm::vec3(5.0f, 1.25f, 2.1f);
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
			t.position = { 0.0f, portalDims.y, -2.2f };
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
			//t.position = { 1.5f, portalDims.y, 3.0f };
			//t.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
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



	// set initial camera position
	{
		Transform t = camera.GetTransform();
		t.position = glm::vec3(1.0f, 0.25, -2.2f - 0.6f);
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

	GLuint fbo;

	ConfigureFBOAndTextureForShadowmap(fbo, shadowMap, stencil_view);
	
	// configure dir light
	light.transform.position = glm::vec3(6.0f, 8.2f, 6.0f);
	light.transform.rotation = glm::vec3(-126.295, -41.2203, 180);


	// testing 
	GLuint cm;
	PrerenderPortal(p4, cm);
	// end testing


	Time::Init();
	//render loop
	while (!glfwWindowShouldClose(window)) {
		Time::Update();
		mc->Update();
		
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

			// render shadowmap
			RenderShadowmap(fbo, light, portalsToRender);

			Cam cam;
			renderDepth = 0;

			// draw the scene the first time
			//if (false)
			{
				// temp
				portallingMat = glm::mat4(1.0f);
				// temp
				cam.worldToView = camera.GetWorldToViewMatrix();
				cam.projection = camera.GetProjectionMatrix();
				sceneMat.shader->Use();
				sceneMat.shader->setUInt("smRef", 0);
				sceneMat.shader->setUInt("smRef2", 7);
				sceneMat.shader->setFloat("dirLight2.intensity", 2.0f);

				// temp
				sceneMat.shader->setMat4("lightSpaceMatrix2", GetLightSpaceMatrix(light) * p5.GetPortallingMat());

				DrawScene(cam);
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

			// divide stencil range by portals
			//if (false)
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
			
			// draw prerendered portal
			if (false)
			{
				Transform t;
				t.position = { 1.0f, portalDims.y, -2.2f };
				//t.rotation = Rotator({ 0.0f, 0.0f, 0.0f });
				Shader* s = ppRenderer->GetMaterial()->GetShader();
				glm::mat4 scale = glm::mat4(1.0f);
				scale[0][0] = 0.5f/portalDims.x;
				scale[1][1] = 0.5f/portalDims.y;
				glm::mat4 scale2 = glm::mat4(1.0f);
				//scale2[0][0] = 8.0f/6.0f;
				//scale2[1][1] = 0.5f / portalDims.y;
				s->Use();
				glm::vec3 psCamPos = scale2 *scale * t.GetInverseTransformMatrix() * glm::vec4(camera.GetTransform().position, 1.0f);
				s->setVec3("psCamPos", psCamPos);
				// do we need this here?
				{
					SetGlobalViewMatrix(cam.worldToView);
					SetGlobalProjectionMatrix(cam.projection);
				}
				//s->setMat4("objectToWorld", t.GetTransformMatrix());
				s->setMat4("portalDimsScaler", scale);
				ppRenderer->Draw();
			}

			// draw portal(s)
			//if (false)
			{
				

				
				unordered_map<const Portal*, glm::mat4> wtvs;

				// set prevStencil  values to zero for each portal
				for (Portal* p : portalsToRender) {
					p->prevStencil = 0;
					wtvs[p] = camera.GetWorldToViewMatrix();
				}


				// set some shader values that don't change during the frame
				glClear(GL_STENCIL_BUFFER_BIT);
				glEnable(GL_STENCIL_TEST);
				while (!portalsToRender.empty()) {

					// render each portal plane with clear color, setting stencil buffer value to portal id
					UpdateStencil(portalsToRender, wtvs);

					glClear(GL_DEPTH_BUFFER_BIT);

					++renderDepth;
					// draw portals
					for (const Portal* p : portalsToRender) {
						cam.worldToView = wtvs[p];
						sceneMat.shader->Use();
						sceneMat.shader->setUInt("smRef", p->stencilVal);
						sceneMat.shader->setUInt("smRef2", p->stencilVal);
						sceneMat.shader->setFloat("dirLight2.intensity", 2.0f);
						wtvs[p] = DrawPortal(*p, cam);
					}

					for (Portal* p : portalsToRender) {
						p->prevStencil = p->stencilVal;
					}

					

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
			DrawPortalPlane(*p, wtvs[p]);
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

		sp->Use();
		sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

		for (auto& m : pmap) {
			//glStencilFuncSeparate(GL_FRONT, GL_EQUAL, m.second.begin()->second->prevStencil, 0xFF);
			//glStencilFuncSeparate(GL_BACK, GL_NEVER, m.second.begin()->second->prevStencil, 0xFF);
			glStencilFunc(GL_EQUAL, m.second.begin()->second->prevStencil, 0xFF);
			for (auto& e : m.second) {
				Portal* p = e.second;
				r = p->GetComponent<MeshRenderer>();
				SetGlobalViewMatrix(wtvs[p]);
				p->stencilVal += 1;
				p->prevStencil += 1;
				r->Draw();
			}
			m.second.erase(m.second.begin());
		}

	}
}

void DrawPortalPlane(const Portal& p, const glm::mat4& worldToView) {
	glStencilFunc(GL_ALWAYS, p.stencilVal, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	MeshRenderer* r = p.GetComponent<MeshRenderer>();
	Shader* sp = r->GetMaterial()->GetShader();

	sp->Use();
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw();
}

glm::mat4 DrawPortal(const Portal& p, const Cam& cam, Material* matOverride) {

	// update world to view matrix 
	glm::mat4 newWTV = cam.worldToView * p.GetPortallingMat();			    

	Cam c = cam;
	c.worldToView = newWTV;

	// draw the scene from a new perspective
	{
		glStencilFunc(GL_EQUAL, p.stencilVal, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		Shader* sp;
		if (matOverride == nullptr)
			// temp
			sp = crateRenderer->GetMaterial()->GetShader();
		else
			sp = matOverride->shader;
		sp->Use();
		sp->setVec4("portalPlaneEq", p.dest->GetViewspacePortalEquation(newWTV));
		
		// temp
		portallingMat = p.GetPortallingMat();

		glEnable(GL_CLIP_DISTANCE0);
		DrawScene(c, matOverride);
		glDisable(GL_CLIP_DISTANCE0);
	}

	return newWTV;
}



void DrawScene(const Cam& cam, Material* matOverride) {
	Shader* sp = nullptr;

	SetGlobalProjectionMatrix(cam.projection);
	SetGlobalViewMatrix(cam.worldToView);

	if (matOverride != nullptr) {
		sp = matOverride->GetShader();
		sp->Use();
	}

	// draw bulb
	if (matOverride == nullptr) {
		sp = bulb.GetComponent<MeshRenderer>()->GetMaterial()->GetShader();
		sp->Use();
		sp->setVec3("lightColor", lightColor);
		sp->setMat4("lightSpaceMatrix", GetLightSpaceMatrix(light));
	}
	bulb.GetComponent<MeshRenderer>()->Draw(matOverride);

	// draw with main shader or override shader:
	{
		// draw monkey
		if (matOverride == nullptr) {
			sp = monkeyRenderer->GetMaterial()->GetShader();
			sp->Use();
			// temp
			sp->setMat4("lightSpaceMatrix", GetLightSpaceMatrix(light) * portallingMat);
			sp->setVec3("lightColor", lightColor);
			sp->setVec3("dirLight.color", lightColor * 0.5f + 0.5f);
			sp->setFloat("dirLight.intensity", 2.0f);
			sp->setVec3("dirLight.direction", glm::mat3(cam.worldToView) * light.transform.rotation.GetForwardVector());
			sp->setFloat("dirLight.ambientStrength", 0.2f);

			// temp
			if (renderDepth == 1)
				sp->setMat4("lightSpaceMatrix2", GetLightSpaceMatrix(light));


			sp->setVec3("dirLight2.color", lightColor * 0.5f + 0.5f);
			sp->setVec3("dirLight2.direction", glm::mat3(cam.worldToView) * light.transform.rotation.GetForwardVector());
			sp->setFloat("dirLight2.ambientStrength", 0.0f);

			sp->setVec3("pointLight.position", cam.worldToView * glm::vec4(bulb.transform.position, 1.0f));
			sp->setFloat("material.shiness", 32.0f);
			glActiveTexture(GL_TEXTURE2);
			sp->setInt("shadowMap", 2);
			glBindTexture(GL_TEXTURE_2D, shadowMap);
			glActiveTexture(GL_TEXTURE3);
			sp->setInt("smStencil", 3);
			glBindTexture(GL_TEXTURE_2D, stencil_view);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
			glActiveTexture(GL_TEXTURE0);
		}
		monkeyRenderer->Draw(matOverride);

		// draw portal scenes
		if (matOverride == nullptr) {
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cam.worldToView * ps1.transform.GetTransformMatrix())));
			sp->SetMat3("normalMatrix", normalMatrix);
		}
		sceneRenderer->Draw(matOverride);
		
		if (matOverride == nullptr) {
			if (renderDepth == 0) {
				sp->setFloat("dirLight.intensity", 0.0f);
				sp->setFloat("dirLight2.intensity", 2.0f);
			}
			else{
				sp->setFloat("dirLight.intensity", 2.0f);
				sp->setFloat("dirLight2.intensity", 0.0f);
			}
			sp->setFloat("dirLight.ambientStrength", 0.0f);
			sp->setFloat("dirLight2.ambientStrength", 0.0f);
			
		}
		if (matOverride == nullptr) {
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cam.worldToView * ps2.transform.GetTransformMatrix())));
			sp->SetMat3("normalMatrix", normalMatrix);
		}
		scene2Renderer->Draw(matOverride);
	}
}

void ConfigureFBOAndTextureForShadowmap(GLuint& fbo, GLuint& tex, GLuint& stencil_view) {
	// https://stackoverflow.com/questions/27535727/opengl-create-a-depth-stencil-texture-for-reading
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
	glGenFramebuffers(1, &fbo);

	// allocate space for the texture and set up a few parameters
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// create stecil view for the texture
	glGenTextures(1, &stencil_view);
	glTextureView(stencil_view, GL_TEXTURE_2D, tex, GL_DEPTH24_STENCIL8, 0, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, stencil_view);
	

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}	

void RenderShadowmap(GLuint fbo, DirLight& light, list<Portal*> portalsToRender) {
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	
	Cam cam = GetLightCam(light);
	SetGlobalProjectionMatrix(cam.projection);
	SetGlobalViewMatrix(cam.worldToView);

	unordered_map<const Portal*, glm::mat4> wtvs;
	

	// set prevStencil  values to zero for each portal
	for (Portal* p : portalsToRender) {
		p->prevStencil = 0;
		wtvs[p] = cam.worldToView;
	}

	{
		// render portal planes first to not render the pixels that will be covered by them anyway ?
		// render the scene first time
		// redner portal planes again to apply stencil values and clear depth
		// for each portal draw the scene again to update the depth map
	}


	// render the scene without portals
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	DrawScene(cam, smMat);
	glDisable(GL_CULL_FACE);
	
	glEnable(GL_STENCIL_TEST);
	// redner portal planes to apply stencil values
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	for (const Portal* p : portalsToRender) {
		glStencilFunc(GL_ALWAYS, p->stencilVal, 0xFF);

		MeshRenderer* r = p->GetComponent<MeshRenderer>();
		r->Draw(smMat);
	}
	
	// draw portal planes again to clear depth where the portals are
	// but only where the stencil is equal to the portal's stencil
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDepthFunc(GL_ALWAYS);
	for (const Portal* p : portalsToRender) {
		glStencilFunc(GL_EQUAL, p->stencilVal, 0xFF);
		
		MeshRenderer* r = p->GetComponent<MeshRenderer>();
		// use shader that sets depth to 1.0 (clears it)
		r->Draw(clearDepthMat);
	}
	glDepthFunc(GL_LESS);
	
	// for each portal draw the scene again to update the depth map
	for (const Portal* p : portalsToRender) {
		cam.worldToView = wtvs[p];
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		wtvs[p] = DrawPortal(*p, cam, smMat);
		glDisable(GL_CULL_FACE);
	}

	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}

glm::mat4 GetLightSpaceMatrix(const DirLight& light) {
	glm::mat4 lv = light.transform.GetInverseTransformMatrix();
	glm::mat4 lp = glm::ortho(-light.extent, light.extent, -light.extent, light.extent,
		light.nearPlane, light.farPlane);
	return lp * lv;
}

Cam GetLightCam(const DirLight& light) {
	Cam cam;
	cam.worldToView = light.transform.GetInverseTransformMatrix();
	cam.projection = glm::ortho(-light.extent, light.extent, -light.extent, light.extent,
		light.nearPlane, light.farPlane);
	return cam;
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
	Cam cam;
	cam.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 100.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	for (GLsizei i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, outCm, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		cam.worldToView = wtvs[i];
		DrawScene(cam);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}


void ApplyLightValuesToShader(const Shader* shader, DirLight light)
{

}


void CreateGlobalMatricesBuffer() {
	globalMatrices = new UniformBufferObject("GlobalMatrices", sizeof(glm::mat4) * 2);
}

void SetGlobalViewMatrix(const glm::mat4& view)
{
	globalMatrices->SetBufferSubData(0, sizeof(glm::mat4), glm::value_ptr(view));
}

void SetGlobalProjectionMatrix(const glm::mat4& projection)
{
	globalMatrices->SetBufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
}