#include "PortalSpace.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Shadows.h"
#include "Portal.h"

std::unordered_set<PortalShadowedDirLight*> PortalSpace::shadowmappedLights;

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

void PortalSpace::Draw(const Cam& cam, Material* matOverride)
{
	SetGlobalProjectionMatrix(cam.projection);
	SetGlobalViewMatrix(cam.worldToView);

	// set global light data here
	lighting->AddLights(this, cam);
	lighting->SendToGPU();

	for (MeshRenderer* mr : renderers)
	{
		if (matOverride == nullptr)
		{
			Shader* sp = mr->GetMaterial()->GetShader();
			sp->Use();
			sp->setInt("shadowMap", 2);
			sp->setInt("smStencil", 3);
			// set shadowmaps for the shader
			// temp
			PortalShadowedDirLight* l = shadowmappedLights.begin().operator*();
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, l->shadowmap->depth_view);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, l->shadowmap->stencil_view);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
			glActiveTexture(GL_TEXTURE0);
			sp->setFloat("material.shiness", 32.0f);
			//sp->setVec3("pointLight.position", cam.worldToView * glm::vec4(bulb.transform.position, 1.0f));



			// set normal matrix
			if (matOverride == nullptr) {
				glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(cam.worldToView * mr->GetTransform().GetTransformMatrix())));
				sp->SetMat3("normalMatrix", normalMatrix);
			}
		}

		// draw
		mr->Draw(matOverride);
	}

	lighting->ClearLights();
}


void PortalSpace::AddPortal(Portal* p)
{
	portals.insert(p);
	p->portalSpace = this;
}

const std::unordered_set<Portal*>& PortalSpace::GetPortals()
{
	return portals;
}