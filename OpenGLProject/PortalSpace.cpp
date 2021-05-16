#include "PortalSpace.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Shadows.h"
#include "Portal.h"
#include "Camera.h"

std::unordered_set<ShadowedDirLight*> PortalSpace::shadowmappedLights;

// temp
extern Actor bulb;

void PortalSpace::AddActor(Actor* actor)
{
	if (actor->portalSpace == this)
		return;
	if (actor->portalSpace != nullptr)
	{
		actor->portalSpace->RemoveActor(actor);
	}
	MeshRenderer* mr = actor->GetComponent<MeshRenderer>();
	if (mr != nullptr)
		renderers.insert(mr);
	actors.insert(actor);
	actor->portalSpace = this;
}

void PortalSpace::RemoveActor(Actor* actor)
{
	if (actor->portalSpace == this)
		actors.erase(actor);
	actor->portalSpace = nullptr;
	MeshRenderer* mr = actor->GetComponent<MeshRenderer>();
	if (mr != nullptr)
		renderers.erase(mr);
}

void PortalSpace::Draw(const Camera* cam, const Material* matOverride)
{
	if (cam != nullptr)
	{
		SetGlobalProjectionMatrix(cam->GetProjectionMatrix());
		SetGlobalViewMatrix(cam->GetWorldToViewMatrix());
	}

	// set global light data here
	if (matOverride == nullptr)
	{
		lighting->AddLights(this, *cam);
		lighting->SendToGPU();
	}

	for (MeshRenderer* mr : renderers)
	{
		Shader* sp;
		if (matOverride == nullptr)
		{
			sp = mr->GetMaterial()->GetShader();
		}
		else
		{
			sp = matOverride->shader;
		}
		sp->Use();
		sp->setFloat("material.shiness", 32.0f);
		// set shadowmaps for the shader
		lighting->SetShadowmaps(sp);


		//temp
		sp->setVec3("pointLight.position", cam->GetWorldToViewMatrix() * glm::vec4(bulb.transform.position, 1.0f));
		sp->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		sp->setVec3("pointLight.position", cam->GetWorldToViewMatrix() * glm::vec4(bulb.transform.position, 1.0f));


		// set normal matrix
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cam->GetWorldToViewMatrix() * mr->GetTransform().GetTransformMatrix())));
		sp->SetMat3("normalMatrix", normalMatrix);

		// draw
		mr->Draw(matOverride);
	}

	if (matOverride == nullptr)
		lighting->ClearLights();
}


void PortalSpace::AddPortal(Portal* p)
{
	portals.insert(p);
	p->portalSpace = this;
}

PortalSpace::PortalContainerConstRef PortalSpace::GetPortals()
{
	return portals;
}