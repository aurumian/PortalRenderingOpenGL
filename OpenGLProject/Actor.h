#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Physics.h"

class Component;
class MeshRenderer;
class Collider;

class Actor
{
public:
	Actor();
	virtual ~Actor();


	void AddComponent(Component* comp);

	template<class T> T* GetComponent() const {
		for (Component* comp : components) {
			T* res = dynamic_cast<T*>(comp);
			if (res != nullptr)
				return res;
		}
		return nullptr;
	}

	Transform transform;
protected:
	virtual void OnRayHit(RayHit hit) {}

	vector<Component*> components;
};

class Portal : public Actor
{
public:
	uint8_t stencilVal;
	uint8_t prevStencil;
	// portals that can be seen through this portal
	vector<Portal*> cbsPortals;

	Portal* dest;

	size_t GetMaxRenderDepth() { return maxRenderDepth; }

	void SetMaxRenderDepth(size_t v) {
		if (v > 0)
			maxRenderDepth = v;
	}

	glm::vec4 GetViewspacePortalEquation(glm::mat4 worldToView) const;

private:
	size_t maxRenderDepth = 1;
};


