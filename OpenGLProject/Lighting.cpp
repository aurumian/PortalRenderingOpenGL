#include "Lighting.h"
#include "UniformBufferObject.h"
#include "SubScene.h"
#include "Shadows.h"
#include "Camera.h"

#include <unordered_map>
#include <string>


Lighting::Lighting()
{
	ubo = new UniformBufferObject("DirLights", sizeof(dirLights));
	dirLights.numDirLights = 0;
}

Lighting::~Lighting() {
	delete ubo;
}

void Lighting::AddLights(const SubScene* ps, const Camera& cam)
{
	unordered_map<ShadowedDirLight*, uint32_t> uniqueShadowedLights;
	for (const DrawableDirLight* ddlp : ps->drawableDirLights)
	{
		if (dirLights.numDirLights >= MAX_DIR_LIGHT_COUNT)
		{
			break;
		}
		const DrawableDirLight& ddl = *ddlp;
		DirLight* light = ddl.sdl->light;
		GpuDirLight& l = dirLights.lights[dirLights.numDirLights++];
		l.direction = glm::mat3(cam.GetWorldToViewMatrix()) * ddl.direction;
		if (ddl.portal == nullptr)
			l.ambientStrength = light->ambientStrenght;
		else
			l.ambientStrength = 0.0f;
		l.color = light->color;
		l.intensity = light->intensity;
		
		l.lightSpaceMatrix = ddl.lightSpaceMatrix;
		l.lsPortalEq = ddl.lsPortalEq;

		l.smStencilRef = ddl.stencilVal;

		// number of uniqueShadowedLights cannot excede MAX_DIR_LIGHT_COUNT
		// but if I'm planning to have different max number of lights
		// and max number of shadowmaps I should include a check here
		// and also lights coming from portals can only be used with 
		// a shadomap
		if (uniqueShadowedLights.find(ddl.sdl) == uniqueShadowedLights.end())
		{
			uniqueShadowedLights.insert({ ddl.sdl, (uint32_t)numShadowmaps });
			shadowmaps[numShadowmaps++] = ddl.sdl->shadowmap;
			
		}
		l.smIndex = uniqueShadowedLights[ddl.sdl];
	}

}

void Lighting::ClearLights()
{
	dirLights.numDirLights = 0;
	numShadowmaps = 0;
}

void Lighting::BindUboToShader(Shader* shader)
{
	ubo->BindToShader(shader);
}

void Lighting::SendToGPU()
{
	ubo->SetBufferSubData(0, sizeof(dirLights), (void*)&dirLights);
}

void Lighting::SetShadowmaps(Shader* shader)
{
	if (shader == nullptr)
		return;
	shader->Use();
	for (size_t i = 0; i < numShadowmaps; ++i)
	{
		int depthViewIndex = (int)i * 2;
		int stencilViewIndex = (int)i * 2 + 1;
		glActiveTexture(GL_TEXTURE0 + depthViewIndex);
		glBindTexture(GL_TEXTURE_2D, shadowmaps[i]->depth_view);
		glActiveTexture(GL_TEXTURE0 + stencilViewIndex);
		glBindTexture(GL_TEXTURE_2D, shadowmaps[i]->stencil_view);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
		glActiveTexture(GL_TEXTURE0);
		shader->setInt(("shadowMap[" + to_string(i) + "]"), depthViewIndex);
		shader->setInt(("smStencil[" + to_string(i) + "]"), stencilViewIndex);
	}
}

glm::mat4 DirLight::GetLightSpaceMatrix() {
	glm::mat4 lv = transform.GetInverseTransformMatrix();
	glm::mat4 lp = glm::ortho(-extent, extent, -extent, extent,
		nearPlane, farPlane);
	return lp * lv;
}

Camera GetLightCam(const DirLight& light) {
	Camera cam;
	cam.SetProjectionMatrixOrtho(-light.extent, light.extent, -light.extent, light.extent,
		light.nearPlane, light.farPlane);
	cam.SetWorldToViewMatrix(light.transform.GetInverseTransformMatrix());
	return cam;
}

