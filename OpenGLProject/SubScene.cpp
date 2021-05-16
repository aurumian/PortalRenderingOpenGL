#include "SubScene.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Shadows.h"
#include "Portal.h"
#include "Camera.h"

std::unordered_set<ShadowedDirLight*> SubScene::shadowmappedLights;

// temp
extern Actor bulb;

void SubScene::AddActor(Actor* actor)
{
	if (actor->subScene == this)
		return;
	if (actor->subScene != nullptr)
	{
		actor->subScene->RemoveActor(actor);
	}
	MeshRenderer* mr = actor->GetComponent<MeshRenderer>();
	if (mr != nullptr)
		renderers.insert(mr);
	actors.insert(actor);
	actor->subScene = this;
}

void SubScene::RemoveActor(Actor* actor)
{
	if (actor->subScene == this)
		actors.erase(actor);
	actor->subScene = nullptr;
	MeshRenderer* mr = actor->GetComponent<MeshRenderer>();
	if (mr != nullptr)
		renderers.erase(mr);
}

void SubScene::Draw(const Camera* cam, const Material* matOverride)
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


void SubScene::AddPortal(Portal* p)
{
	portals.insert(p);
	p->subScene = this;
}

SubScene::PortalContainerConstRef SubScene::GetPortals()
{
	return portals;
}