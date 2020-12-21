#pragma once

#include "MathInclude.h"
#include "Rotator.h"
#include <string>

class Transform
{
protected:
	
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	Rotator rotation;

	Transform();
	~Transform();

	glm::vec3 GetScale() const {
		return scale;
	}

	void SetScale(glm::vec3 newScale) {
		scale.x = glm::max(newScale.x, 0.0f);
		scale.y = glm::max(newScale.y, 0.0f);
		scale.z = glm::max(newScale.z, 0.0f);
	}

	glm::mat4 GetTransformMatrix() const;

	glm::mat4 GetInverseTransformMatrix() const;

	glm::mat3 GetNormalMatrix() const;

	std::string ToString() const;
};

