#pragma once

#include "MathInclude.h"
#include "Component.h"

#include <vector>

#include "Transform.h"

class Collider;
struct Ray;
struct RayHit;
class Pyramid;
class RectanglePlane;


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

	static bool IsPointInside(const Pyramid& pyramid, const glm::vec3& point);
	
	static bool CheckOverlap(const Pyramid& p1, const Pyramid& p2);

	static bool CheckOverlap(const Pyramid& pyramid, const RectanglePlane& plane);

	static bool Overlap1D(const glm::vec2& range1, const glm::vec2& range2);

	static std::pair<float, bool> GetDistanceToPlaneAlongInfiniteRay(const Ray& ray, const glm::vec3& normal, const glm::vec3& planePoint);
	
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

struct SimpleRayHit
{
	glm::vec3 hitPosition;
	bool didHit = false;
};

class Pyramid
{
public:
	static const size_t NUM_BASE_POINTS = 4;
	static const size_t NUM_POINTS = NUM_BASE_POINTS + 1;
	static const size_t NUM_EQS = NUM_POINTS;
	static const size_t APEX_INDEX = NUM_POINTS - 1;
	static const size_t BASE_EQ_INDEX = APEX_INDEX;
	/*
	* points - array of points on each of the 4 apex to base vectors
	*/
	Pyramid(const glm::vec3 basePoints[NUM_BASE_POINTS], const glm::vec3& apex);
	Pyramid();

	bool IsPointInside(const glm::vec3& point) const;
	bool IsPointInsideInfinite(const glm::vec3& point) const;

	bool DoesRayIntersect(const Ray& ray) const;


	// expects the points in clockwise order
	void Construct(const glm::vec3 basePoints[NUM_BASE_POINTS], const glm::vec3& apex);

private:
	// normals of the plane's of the infinite pyramid facing inside
	glm::vec4 planeEqs[NUM_EQS];
	glm::vec3 points[NUM_POINTS];
	glm::vec3 right;
	glm::vec3 up;
	glm::vec2 extent;

	bool DoesIntersectSideFace(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& normal, const Ray& ray) const;
	bool DoesIntersectSideFace(size_t index, const Ray& ray) const;
	bool DoesIntersectBaseFace(const Ray& ray) const;

	friend class Physics;
};

class RectanglePlane
{
public:
	const static size_t NUM_POINTS = 4;
	RectanglePlane(const glm::vec3& origin, const glm::vec3& normal, const glm::vec2& extent);

	bool DoesRayHit(const Ray& ray) const;
private:
	glm::vec3 points[NUM_POINTS];
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 forward;
	glm::vec2 extent;
	glm::vec3 origin;

	friend class Physics;
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
