#pragma once
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Rotator
{
protected:
	// quaternion representation of rotation
	glm::fquat quat;

public: 
	Rotator();
	Rotator(const glm::vec3 eulerAngles);
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

