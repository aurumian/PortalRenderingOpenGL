#include "Common.h"
#include "Material.h"

#include "UniformBufferObject.h"
#include "Actor.h"
#include "MeshRenderer.h"
#include "Portal.h"
#include "Lighting.h"
#include "Shadows.h"
#include "PortalSpace.h"

UniformBufferObject* globalMatrices;
int screenWidth = 800, screenHeight = 600;
Material sceneMat;

PortalSpace defaultPortalSpace;

PortalSpace* currentPortalSpace;




// temp
glm::mat4 portallingMat;

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

PortalSpace* GetDefaultPortalSpace()
{
	return &defaultPortalSpace;
}