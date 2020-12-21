#include "Physics.h"


Physics::Physics()
{
	if (instance != nullptr)
		throw "Physics is a singleton";
	instance = this;

}


Physics::~Physics()
{
}

Physics* Physics::instance = nullptr;

RayHit Physics::RayCast(const Ray& ray) {
	RayHit hit;
	hit.hitSmth = false;
	float minDist = -1.0f;
	for (Collider* c : colliders) {
		RayHit h = c->RayCast(ray);
		if (h.hitSmth && (minDist < 0.0f || h.hitDistance < minDist)) {
			hit = h;
			minDist = h.hitDistance;
		}
	}
	return hit;
}


RayHit PlaneCollider::RayCast(const Ray& ray) {
	RayHit hit;
	hit.hitSmth = false;
	if (ray.direction == glm::vec3(0.0f, 0.0f, 0.0f))
		return hit;

	Transform transform = GetTransform();
	
	// point on a plane
	glm::vec3 o = transform.position;
	// make sure ray can hit the plane
	if (glm::dot(o - ray.origin, ray.direction) > 0) {
		// plane's normal
		glm::vec3 normal = transform.GetNormalMatrix() * glm::vec3(0.0f, 0.0f, 1.0f);
		if (glm::dot(ray.direction, normal) > 0)
			normal = -normal;

		// distance from ray.origin to plane in the direction of ray.direction;
		float distance = glm::dot((o - ray.origin), normal) / glm::dot(ray.direction, normal);

		// the point of intersection between ray and the plane
		glm::vec3 p = ray.origin + distance * ray.direction;

		hit.ray = ray;
		hit.hitLocation = p;
		hit.hitDistance = distance;
		hit.hitNormal = normal;
		hit.collider = this;
		p = transform.GetInverseTransformMatrix() * glm::vec4(p, 1.0f);
		hit.hitSmth =	distance <= ray.maxDistance &&
						p.x >= -halfDims.x && p.x <= halfDims.x &&
						p.y >= -halfDims.y && p.y <= halfDims.y;
	}

	return hit;
}