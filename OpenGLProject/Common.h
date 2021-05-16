#pragma once

#include "MathInclude.h"

#include <vector>
#include <unordered_set>

class UniformBufferObject;
class Portal;
class Material;
class DirLight;
class Actor;
class MeshRenderer;
struct PortalShadowedDirLight;
class Lighting;
struct DrawableDirLight;
class SubScene;
class Camera;
class PortalRenderTree;

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// most of this file's contents is temporary stuff

//struct Cam {
//	glm::mat4 worldToView;
//	glm::mat4 projection;
//	float nearPlane = 0.0f;
//	bool isOrtho = false;
//};

extern UniformBufferObject* globalMatrices;

extern Material sceneMat;

extern Lighting* lighting;

extern Material gPassMat;
extern Material gPassInsMat;

// temp
extern glm::mat4 portallingMat;

void CreateGlobalMatricesBuffer();

void CreatePortalBlockUbo();

void SetGlobalViewMatrix(const glm::mat4& view);

void SetGlobalProjectionMatrix(const glm::mat4& projection);

void SetGlobalViewspacePortalEquation(const glm::vec4& eq);

void SetGlobalClippingPlane2(const glm::vec4& eq);

void DrawScene(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride = nullptr);

void ForwardRenderScene(const Camera& camera);


void CreateGBuffer();

void DeferedRenderScene(const Camera& camera);

void GeometryPass(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride);

UniformBufferObject* GetPortalBlockUbo();

// if matOverride is not nullptr DrawScene doesn't change shader that is used
// nor does it set any shader parameters except objectToWorld and cam matrices
// void DrawScene(const Cam& cam, Material* matOverride = nullptr);


extern int screenWidth, screenHeight;

class Shadows;

extern Shadows* shadows;

extern SubScene* currentSubScene;

SubScene* GetDefaultSubScene();

struct InBetweenObject
{
	Actor* actor;
	glm::vec3 enteredNormal;
	Portal* enteredPortal;
};


void SetCommonShaderValues(MeshRenderer* mr, const Camera& camera, const Material* matOverride = nullptr);