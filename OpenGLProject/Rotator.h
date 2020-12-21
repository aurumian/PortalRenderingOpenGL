#pragma once
#include "MathInclude.h"

class Rotator
{
protected:
	// quaternion representation of rotation
	glm::fquat quat;

public: 
	Rotator();
	Rotator(const glm::vec3 eulerAngles);
	Rotator(const glm::fquat quat);
	~Rotator();

	// set rotation with euler angles in degrees
	void SetEulerAngles(const glm::vec3 eulerAngles);

	glm::vec3 GetEulerAngles() const;

	glm::vec3 GetEulerRadians() const;

	glm::vec3 GetForwardVector() const;

	glm::vec3 GetUpVector() const;

	glm::vec3 GetRightVector() const;

	// rotates angle degrees around given axis
	void RotateArounAxis(float angle, const glm::vec3 axis);

	glm::fquat GetQuaterion() const {
		return quat;
	}


};

