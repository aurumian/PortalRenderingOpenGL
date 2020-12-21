#include "Rotator.h"

Rotator::Rotator()
{
	SetEulerAngles(glm::vec3(0.0f, 0.0f , 0.0f));
}

Rotator::Rotator(const glm::vec3 eulerAngles)
{
	SetEulerAngles(eulerAngles);
}

Rotator::Rotator(const glm::fquat quat)
{
	this->quat = quat;
}


Rotator::~Rotator()
{
}

void Rotator::SetEulerAngles(const glm::vec3 eulerAngles)
{
	//translate degrees to radians
	glm::vec3 euler  = glm::radians(eulerAngles);
	quat = glm::fquat(euler);
}

glm::vec3 Rotator::GetEulerAngles() const
{
	return glm::degrees(glm::eulerAngles(quat));
}

glm::vec3 Rotator::GetEulerRadians() const
{
	return glm::eulerAngles(quat);
}

glm::vec3 Rotator::GetForwardVector() const
{
	return quat * glm::vec3(0.0f, 0.0f, 1.0f);
}



glm::vec3 Rotator::GetUpVector() const
{
	glm::vec3 euler = GetEulerRadians();
	euler.x += glm::radians(90.0f);
	glm::vec3 up;
	up.x = sin(euler.x) * sin(euler.z);
	up.y = sin(euler.x) * cos(euler.z);
	up.z = cos(euler.x);
	return quat * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Rotator::GetRightVector() const
{
	return quat * glm::vec3(1.0f, 0.0f, 0.0f);
}

void Rotator::RotateArounAxis(float angle, const glm::vec3 axis)
{
	quat = glm::rotate(quat, glm::radians(angle), glm::normalize(axis));
}
