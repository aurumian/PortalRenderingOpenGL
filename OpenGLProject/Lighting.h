#pragma once

#include <glad/glad.h>
#include <unordered_set>

#include "Transform.h"
#include "Common.h"
#include "Actor.h"



class Shader;
class UniformBufferObject;
class DirLight;
struct ShadowedDirLight;
class ShadowmapTexture;
class Camera;
struct DrawableDirLigth;

// mapping of the GPU side DirLight struct onto CPU
class GpuDirLight {
public:
	glm::vec3 direction;
	float ambientStrength;
	glm::vec3 color;
	float intensity;
	uint32_t smStencilRef;
	uint32_t smIndex;
	float _pad[2];
	glm::mat4 lightSpaceMatrix;
	glm::vec4 lsPortalEq; // lightspace portal equation - for better shadow sampling
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

	void AddLights(const SubScene* ps, const Camera& cam);

	void ClearLights();

	void BindUboToShader(Shader* shader);

	void SendToGPU();

	void SetShadowmaps(Shader* shader);

protected:
	UniformBufferObject* ubo;
	GpuDirLights<MAX_DIR_LIGHT_COUNT> dirLights;

	ShadowmapTexture* shadowmaps[MAX_DIR_LIGHT_COUNT];
	size_t numShadowmaps;

	friend void DrawScene(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride);
	friend void GeometryPass(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride);
};

class DirLight : public Actor
{
public:
	float intensity = 1.0f;
	float ambientStrenght = 0.1f;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	GLuint shadowmap;

	// shadow rendering parameters
	float nearPlane = 0.0f;
	float farPlane = 20.0f;
	// height and width of the rendering frustum
	float extent = 20.0f;

	glm::mat4 GetLightSpaceMatrix();
};

Camera GetLightCam(const DirLight& light);



struct ShadowedDirLight
{
	DirLight* light;
	ShadowmapTexture* shadowmap;
	std::unordered_set<DrawableDirLight*> drawableLights;
};

struct DrawableDirLight
{
	ShadowedDirLight* sdl;
	uint32_t stencilVal;
	glm::mat4 lightSpaceMatrix;
	glm::vec3 direction;
	glm::vec4 lsPortalEq;
	Portal* portal;
};