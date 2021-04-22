#include "Lighting.h"
#include "UniformBufferObject.h"
#include "PortalSpace.h"

Lighting::Lighting()
{
	ubo = new UniformBufferObject("DirLights", sizeof(dirLights));
	dirLights.numDirLights = 0;
}

Lighting::~Lighting() {
	delete ubo;
}

void Lighting::AddLights(const PortalSpace* ps, const Cam& cam)
{
	for (const DrawableDirLight* ddlp : ps->drawableDirLights)
	{
		const DrawableDirLight& ddl = *ddlp;
		DirLight* light = ddl.psdl->light;
		if (dirLights.numDirLights >= MAX_DIR_LIGHT_COUNT)
		{
			break;
		}
		PerPortalDirLightData& data = ddl.psdl->perPortal[ddl.portal];
		GpuDirLight& l = dirLights.lights[dirLights.numDirLights++];
		l.direction = glm::mat3(cam.worldToView) * data.direction;
		l.ambientStrength = light->ambientStrenght;
		l.color = light->color;
		l.intensity = light->intensity;
		l.smStencilRef = data.stencilVal;
		l.lightSpaceMatrix = data.lightSpaceMatrix;
	}

}

void Lighting::ClearLights()
{
	dirLights.numDirLights = 0;
}

void Lighting::BindUboToShader(Shader* shader)
{
	ubo->BindToShader(shader);
}

void Lighting::SendToGPU()
{
	ubo->SetBufferSubData(0, sizeof(dirLights), (void*)&dirLights);
}

glm::mat4 DirLight::GetLightSpaceMatrix() {
	glm::mat4 lv = transform.GetInverseTransformMatrix();
	glm::mat4 lp = glm::ortho(-extent, extent, -extent, extent,
		nearPlane, farPlane);
	return lp * lv;
}

Cam GetLightCam(const DirLight& light) {
	Cam cam;
	cam.worldToView = light.transform.GetInverseTransformMatrix();
	cam.projection = glm::ortho(-light.extent, light.extent, -light.extent, light.extent,
		light.nearPlane, light.farPlane);
	return cam;
}

