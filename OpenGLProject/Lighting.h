#pragma once

#include <glad/glad.h>
#include <map>

#include "Transform.h"
#include "Common.h"
#include "Actor.h"



class Shader;
class UniformBufferObject;
class DirLight;
struct PerPortalDirLightData;
class PortalShadowedDirLight;
class ShadowmapTexture;

// mapping of the GPU side DirLight struct onto CPU
class GpuDirLight {
public:
	glm::vec3 direction;
	float ambientStrength;
	glm::vec3 color;
	float intensity;
	uint32_t smStencilRef;
	float _pad[3];
	glm::mat4 lightSpaceMatrix;
};

template<size_t N>
struct GpuDirLights
{
	GpuDirLight lights[N];
	uint32_t numDirLights;
	float _pad[3];
};

class Lighting
{
public:
	static const size_t MAX_DIR_LIGHT_COUNT = 4;

	Lighting();

	~Lighting();

	void AddLights(const PortalSpace* ps, const Cam& cam);

	void ClearLights();

	void BindUboToShader(Shader* shader);

	void SendToGPU();

protected:
	UniformBufferObject* ubo;
	GpuDirLights<MAX_DIR_LIGHT_COUNT> dirLights;
};

class DirLight : public Actor
{
public:
	float intensity = 1.0f;
	float ambientStrenght = 0.1f;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	GLuint smStencil = 0;
	glm::mat4 lightSpaceMatrix; // matrix that transforms worldspace coordinates to lightspace
	GLuint shadowmap;

	// shadow rendering parameters
	float nearPlane = 1.0f;
	float farPlane = 20.0f;
	// height and width of the rendering frustum
	float extent = 20.0f;

	glm::mat4 GetLightSpaceMatrix();
};

Cam GetLightCam(const DirLight& light);

/*
* needs to take into account all the lights that exist in the space
* and all the lights that are coming into the space
*/
std::vector<PortalShadowedDirLight> GetVisibleDirLights(PortalSpace* space);

struct PerPortalDirLightData {
	uint32_t stencilVal;
	glm::mat4 lightSpaceMatrix;
	glm::vec3 direction;
};

class PortalShadowedDirLight
{
public:
	DirLight* light;
	ShadowmapTexture* shadowmap;
	std::map<Portal*, PerPortalDirLightData> perPortal;
};

class DrawableDirLight
{
public:
	PortalShadowedDirLight* psdl;
	// to query perPortal of PortalShadowedDirLight
	Portal* portal;
};