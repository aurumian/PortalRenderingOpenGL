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
class DrawableDirLight;
class Lighting;

// most of this file's contents is temporary stuff

struct Cam {
	glm::mat4 worldToView;
	glm::mat4 projection;
};

class PortalSpace 
{
public:
	// all drawable lights of a portal space
	std::unordered_set<DrawableDirLight*> drawableLights;

	std::unordered_set<DirLight*> dirLights;

	void AddActor(Actor* actor);

	void RemoveActor(Actor* actor);

	void Draw(const Cam& cam, Material* matOverride = nullptr);

	void AddPortal(Portal* p);

	const std::unordered_set<Portal*>& GetPortals();

protected:
	std::unordered_set<Actor*> actors;
	std::unordered_set<MeshRenderer*> renderers;
	std::unordered_set<Portal*> portals;
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

PortalSpace* GetDefaultPortalSpace();