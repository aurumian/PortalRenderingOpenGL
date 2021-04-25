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
class PortalShadowedDirLight;
class Lighting;
class DrawableDirLight;
class PortalSpace;

// most of this file's contents is temporary stuff

struct Cam {
	glm::mat4 worldToView;
	glm::mat4 projection;
	float nearPlane = 0.0f;
	bool isOrtho = false;
};

extern UniformBufferObject* globalMatrices;

extern Material sceneMat;

extern Lighting* lighting;

// temp
extern glm::mat4 portallingMat;

void CreateGlobalMatricesBuffer();

void SetGlobalViewMatrix(const glm::mat4& view);

void SetGlobalProjectionMatrix(const glm::mat4& projection);

// if matOverride is not nullptr DrawScene doesn't change shader that is used
// nor does it set any shader parameters except objectToWorld and cam matrices
void DrawScene(const Cam& cam, Material* matOverride = nullptr);


extern int screenWidth, screenHeight;

class Shadows;

extern Shadows* shadows;

extern PortalSpace* currentPortalSpace;

PortalSpace* GetDefaultPortalSpace();