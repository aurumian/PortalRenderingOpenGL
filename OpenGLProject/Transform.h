#pragma once
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Rotator.h"

class Transform
{
protected:
	
	glm::vec3 scale;

public:
	glm::vec3 position;
	Rotator rotation;

	Transform();
	~Transform();

	glm::vec3 GetScale() const {
		return scale;
	}
};

