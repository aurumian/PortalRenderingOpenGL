#include "Transform.h"
#include <string>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

Transform::Transform()
{
}


Transform::~Transform()
{
}

glm::mat4 Transform::GetTransformMatrix() const
{
	glm::mat4 res = glm::mat4(1.0f);
	res = glm::translate(res, position);
	res = res * glm::mat4_cast(rotation.GetQuaterion());
	res = glm::scale(res, scale);
	return res;
}

glm::mat4 Transform::GetInverseTransformMatrix() const
{
	return glm::inverse(GetTransformMatrix());
}

glm::mat3 Transform::GetNormalMatrix() const
{
	return glm::transpose(glm::inverse(glm::mat3(GetTransformMatrix())));
}

std::string Transform::ToString() const
{
	stringstream ss;
	ss << "Position: " << position.x << ", " << position.y << ", " << position.z << endl;
	ss << "Scale: " << scale.x << ", " << scale.y << ", " << scale.z << endl;
	glm::vec3 euler = rotation.GetEulerAngles();
	ss << "Rotation: " << euler.x << ", " << euler.y << ", " << euler.z << endl;

	return ss.str();
}