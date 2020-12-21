#include "Component.h"
#include "Actor.h"

Transform Component::GetTransform() {
	if (owner != nullptr)
		return owner->transform;
	else
		return Transform();
}