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


glm::vec4 Portal::GetViewspacePortalEquation(glm::mat4 worldToView) const
{
	glm::vec3 normal = glm::transpose(glm::inverse(glm::mat3(worldToView * transform.GetTransformMatrix()))) * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 q = worldToView * glm::vec4(transform.position, 1.0f);
	// make sure (pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	if (glm::dot(normal, q) < 0)
		normal = -normal;
	return glm::vec4(normal, -glm::dot(normal, q));
	// this equation can be used to check a point position around this portal:
	// if (pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	// else it's between the plane and camera's far plane
}
