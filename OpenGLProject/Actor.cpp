#include "Actor.h"
#include "Physics.h"


Actor::Actor()
{
}


Actor::~Actor()
{
	for (auto* c : components) {
		delete c;
	}
}

void Actor::AddComponent(Component* comp)
{
	if (comp != nullptr) {
		components.push_back(comp);
		comp->owner = this;
	}
}

SubScene* Actor::GetSubScene() const
{
	return subScene;
}

