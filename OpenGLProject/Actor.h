#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Physics.h"
#include "Common.h"
#include <list>

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

	SubScene* GetSubScene() const;

	Transform transform;

	// TODO: move this to private
	SubScene* subScene;
protected:
	virtual void OnRayHit(RayHit hit) {}

	vector<Component*> components;

private:
	
	friend class SubScene;
};
