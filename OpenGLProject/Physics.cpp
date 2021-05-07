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
	
	// plane's normal
	glm::vec3 normal = transform.GetNormalMatrix() * glm::vec3(0.0f, 0.0f, 1.0f);

	// point on a plane
	glm::vec3 o = transform.position;
	// make sure ray can hit the plane
	float q_rdotn = glm::dot(o - ray.origin, normal);
	float dirdotn = glm::dot(ray.direction, normal);
	if (q_rdotn >= 0.0f && dirdotn <= 0.0f ||
		q_rdotn <= 0.0f && dirdotn >= 0.0f)
		return hit;


	if (glm::dot(ray.direction, normal) > 0.0f)
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

	return hit;
}

bool Physics::IsPointInside(const Pyramid& pyramid, const glm::vec3& point)
{
	return pyramid.IsPointInside(point);
}

bool Physics::CheckOverlap(const Pyramid& p1, const Pyramid& p2)
{
	for (size_t i = 0; i < Pyramid::NUM_POINTS; ++i)
	{
		if (IsPointInside(p1, p2.points[i]))
			return true;
	}

	for (size_t i = 0; i < Pyramid::NUM_POINTS; ++i)
	{
		if (IsPointInside(p2, p1.points[i]))
			return true;
	}

	return false;
}

// right now the planes that are fully behind the base plane(but inside the infinite pyramid) are considered overlapping
// if we check the point againgst a non-infinite pyramid then there's a case when the plane is not consiredered
// to be ovelapping the pyramid, but should be
// the case is when the plane intersects the base plane and the side plane, but doesn't have any points inside the pyramid(non-infinite)
// one solution is to check whether the plane intersect the base plane within coordinates
//
bool Physics::CheckOverlap(const Pyramid& pyramid, const RectanglePlane& plane)
{
	// a pyramid and a plane overlap if at least one plane's point is insied the pyramid
	for (size_t i = 0; i < RectanglePlane::NUM_POINTS; ++i)
	{
		if (pyramid.IsPointInside(plane.points[i]))
			return true;
	}

	// a pyramid and a plane overlap if at least one edge of the pyramid intersect the plane
	for (size_t i = 0; i < Pyramid::NUM_BASE_POINTS; ++i) // edges of side faces
	{
		Ray ray;
		ray.origin = pyramid.points[Pyramid::APEX_INDEX];
		ray.direction = pyramid.points[i] - pyramid.points[Pyramid::APEX_INDEX];
		ray.maxDistance = glm::length(ray.direction);
		ray.direction /= ray.maxDistance;
		if (plane.DoesRayHit(ray))
			return true;
	}
	for (size_t i = 0; i < Pyramid::NUM_BASE_POINTS; ++i) // edges of base face
	{
		Ray ray;
		ray.origin = pyramid.points[i];
		ray.direction = pyramid.points[(i-1) % Pyramid::NUM_BASE_POINTS] - pyramid.points[i];
		ray.maxDistance = glm::length(ray.direction);
		ray.direction /= ray.maxDistance;
		if (plane.DoesRayHit(ray))
			return true;
	}

	// a pyramid and a rectangle overlap if at least one edge of the rectangle intesects the pyramid
	for (size_t i = 0; i < RectanglePlane::NUM_POINTS; ++i)
	{
		Ray ray;
		ray.origin = plane.points[i];
		ray.direction = plane.points[(i - 1) % RectanglePlane::NUM_POINTS] - plane.points[i];
		ray.maxDistance = glm::length(ray.direction);
		ray.direction /= ray.maxDistance;
		if (pyramid.DoesRayIntersect(ray))
			return true;
	}
	
	return false;
}

bool Physics::Overlap1D(const glm::vec2& range1, const glm::vec2& range2)
{
	return range1.y >= range2.x && range2.y >= range1.x;
}

Pyramid::Pyramid(const glm::vec3 points[NUM_BASE_POINTS], const glm::vec3& apex)
{
	Construct(points, apex);
}

Pyramid::Pyramid()
{
	glm::vec3 basePoints[NUM_BASE_POINTS] = { glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, -1.0f, 1.0f) };
	Construct(basePoints, glm::vec3(0.0f, 0.0f, 0.0f));
}

bool Pyramid::IsPointInside(const glm::vec3& point) const
{
	glm::vec4 p(point, 1.0f);
	for (int i = 0; i < NUM_EQS; ++i)
	{
		if (glm::dot(p, planeEqs[i]) < 0.0f)
			return false;
	}

	return true;
}

bool Pyramid::IsPointInsideInfinite(const glm::vec3& point) const
{
	glm::vec4 p(point, 1.0f);
	for (int i = 0; i < NUM_EQS - 1; ++i)
	{
		if (glm::dot(p, planeEqs[i]) < 0.0f)
			return false;
	}

	return true;
}

bool Pyramid::DoesRayIntersect(const Ray& ray) const
{
	// check side face
	for (size_t i = 0; i < NUM_BASE_POINTS; ++i)
	{
		if (DoesIntersectSideFace(i, ray))
			return true;
	}

	// check base face
	return DoesIntersectBaseFace(ray);
}

std::pair<float, bool> Physics::GetDistanceToPlaneAlongInfiniteRay(const Ray& ray, const glm::vec3& normal, const glm::vec3& planePoint)
{
	if (ray.direction == glm::vec3(0.0f, 0.0f, 0.0f))
		return { 0.0f, false };

	float dirdotn = glm::dot(ray.direction, normal);
	float q_odotn = glm::dot((planePoint - ray.origin), normal);
	// the ray is parallel to the plane
	if (dirdotn == 0.0f)
	{
		// the point might be on a plane
		return { q_odotn, q_odotn == 0.0f };
	}


	// distance from ray.origin to plane in the direction of ray.direction;
	float distance = q_odotn / dirdotn;

	return { distance, true };
}

void Pyramid::Construct(const glm::vec3 basePoints[NUM_BASE_POINTS], const glm::vec3& apex)
{
	std::copy(basePoints, basePoints + NUM_BASE_POINTS, points);
	this->points[APEX_INDEX] = apex;

	// get a point inside the pyramid 
	// the following is equivalent to sum(point[i] - apex)/NUM_BASE_POINTS/2.0+apex
	glm::vec3 insPoint = points[0];
	for (size_t i = 1; i < NUM_BASE_POINTS; ++i)
		insPoint += points[i];
	insPoint *= 1.0f / float(NUM_BASE_POINTS * 2);
	insPoint = insPoint + apex * 0.5f;
	glm::vec4 insidePoint(insPoint, 1.0f);

	// calculate pyramid's plane equations
	glm::vec3 baseNormal = glm::normalize(glm::cross(points[2] - points[1], points[0] - points[1]));
	planeEqs[BASE_EQ_INDEX] = glm::vec4(baseNormal, -glm::dot(baseNormal, points[0]));
	for (size_t i = 0; i < NUM_BASE_POINTS; ++i)
	{
		glm::vec3 v1 = points[i] - apex;
		glm::vec3 v2 = points[(i + 1) % NUM_BASE_POINTS] - apex;
		glm::vec3 normal = glm::normalize(glm::cross(points[(i+1) % NUM_BASE_POINTS] - apex, points[i] - apex));
		planeEqs[i] = glm::vec4(normal, -glm::dot(normal, apex));
	}

	// calculate right and up vectors
	{
		right = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), -baseNormal);

		// if normal and up vector are linearly dependant
		// we get the zero right vector
		if (right == glm::vec3())
		{
			up = glm::normalize(glm::cross(baseNormal, glm::vec3(1.0f, 0.0f, 0.0f)));
			right = glm::cross(up, baseNormal); // don't need to normalize since forward and up are peppendicular
		}
		else
		{
			up = glm::cross(baseNormal, right); // don't need to normalize since forward and up are peppendicular
		}
	}

	// calculate extent
	extent.x = glm::abs(glm::dot(points[2], right));
	extent.y = glm::abs(glm::dot(points[2], up));
}

bool Pyramid::DoesIntersectSideFace(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& normal, const Ray& ray) const
{
	auto res = Physics::GetDistanceToPlaneAlongInfiniteRay(ray, normal, p0);
	if (!res.second)
		return false;
	float distance = res.first;

	if (distance > ray.maxDistance)
		return false;

	if (distance < 0.0f)
		return false;

	// get intersection point
	glm::vec3 point = ray.origin + distance * ray.direction;

	// check that the point is withing the triangle
	return glm::dot(normal, glm::cross(point - p0, p1 - p0)) > 0.0f &&
		   glm::dot(normal, glm::cross(point - p1, p2 - p1)) > 0.0f &&
		   glm::dot(normal, glm::cross(point - p2, p0 - p2)) > 0.0f
		;
}

bool Pyramid::DoesIntersectSideFace(size_t index, const Ray& ray) const
{
	return DoesIntersectSideFace(points[APEX_INDEX], points[index], points[(index + 1) % NUM_BASE_POINTS], planeEqs[index], ray);
}

bool Pyramid::DoesIntersectBaseFace(const Ray& ray) const
{
	auto res = Physics::GetDistanceToPlaneAlongInfiniteRay(ray, planeEqs[BASE_EQ_INDEX], points[0]);

	float distance = res.first;
	if (!res.second)
		return false;

	// distance must be positive
	if (distance < 0.0f)
		return false;

	if (distance > ray.maxDistance)
		return false;

	// the point of intersection between ray and the plane
	glm::vec3 p = ray.origin + distance * ray.direction;

	glm::vec3 o = points[0] + points[1] + points[2] + points[3];
	o /= 4.0f;

	// check if the point of intersection is inside the plane's borders
	float x = glm::dot(p - o, right);
	float y = glm::dot(p - o, up);

	return distance <= ray.maxDistance &&
		x >= -extent.x && x <= extent.x &&
		y >= -extent.y && y <= extent.y;
	return false;
}

RectanglePlane::RectanglePlane(const glm::vec3& origin, const glm::vec3& normal, const glm::vec2& extent)
{
	const glm::vec3 zero(0.0f, 0.0f, 0.f);

	this->extent = extent;
	this->origin = origin;

	if (normal == zero)
		forward = glm::vec3(0.0f, 0.0f, 1.0f);
	else
		forward = glm::normalize(normal);

	right = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward);

	// if normal and up vector are linearly dependant
	// we get the zero right vector
	if (right == glm::vec3())
	{
		up = glm::normalize(glm::cross(forward, glm::vec3(1.0f, 0.0f, 0.0f)));
		right = glm::cross(up, forward); // don't need to normalize since forward and up are peppendicular
	}
	else
	{
		up = glm::cross(forward, right); // don't need to normalize since forward and up are peppendicular
	}

	points[0] = origin - right * extent.x - up * extent.y;
	points[1] = origin - right * extent.x + up * extent.y;
	points[2] = origin + right * extent.x + up * extent.y;
	points[3] = origin + right * extent.x - up * extent.y;
}

bool RectanglePlane::DoesRayHit(const Ray& ray) const
{
	auto res = Physics::GetDistanceToPlaneAlongInfiniteRay(ray, forward, points[0]);

	float distance = res.first;
	if (!res.second)
		return false;

	// distance must be positive
	if (distance < 0.0f)
		return false;

	// the point of intersection between ray and the plane
	glm::vec3 p = ray.origin + distance * ray.direction;

	// check if the point of intersection is inside the plane's borders
	p = p - origin;
	float x = glm::dot(p, right);
	float y = glm::dot(p, up);

	return distance <= ray.maxDistance && 
		x >= -extent.x && x <= extent.x &&
		   y >= -extent.y && y <= extent.y;
}
