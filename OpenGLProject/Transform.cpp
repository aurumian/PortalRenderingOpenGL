#include "Transform.h"

Transform::Transform()
{
}


Transform::~Transform()
{
}

glm::mat4 Transform::GetObjectToWorldMatrix() const
{
	glm::mat4 res = glm::mat4(1.0f);
	res = glm::translate(res, position);
	res = res * glm::mat4_cast(rotation.GetQuaterion());
	res = glm::scale(res, scale);
	return res;
}
