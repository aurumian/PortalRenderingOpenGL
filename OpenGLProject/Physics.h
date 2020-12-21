#pragma once

#include "MathInclude.h"
#include "Component.h"

#include <vector>

#include "Transform.h"

class Collider;
struct Ray;
struct RayHit;


class Actor;

class Physics
{
public:
	Physics();
	~Physics();

	static Physics* Instance() {
		return instance;
	}

	void AddCollider(Collider* collider) {
		if (collider != nullptr) {
			colliders.push_back(collider);
		}
	}
	
	
	RayHit RayCast(const Ray& ray);
private:
	static Physics* instance;
	std::vector<Collider*> colliders;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
	float maxDistance;
};

struct RayHit {
	Ray ray;
	glm::vec3 hitNormal;
	glm::vec3 hitLocation;
	float hitDistance;
	bool hitSmth;
	Collider* collider;
};


class Collider : public Component {
public:
	virtual RayHit RayCast(const Ray& ray) = 0;

	bool isTrigger = false;

	Collider() {

	}

	Collider(bool isTrigger):isTrigger(isTrigger){}

	virtual ~Collider() {}
};

class PlaneCollider : public Collider{
public:
	virtual RayHit RayCast(const Ray& ray) override;
	glm::vec2 halfDims;
};
