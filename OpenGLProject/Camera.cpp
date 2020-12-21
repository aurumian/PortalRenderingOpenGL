#include "Camera.h"

Camera::Camera()
{
}


Camera::~Camera()
{
}

glm::mat4 Camera::GetWorldToViewMatrix() const
{
	//return glm::lookAt(transform.position, transform.position + transform.rotation.GetForwardVector(), transform.rotation.GetUpVector());
	return transform.GetInverseTransformMatrix();
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	//if (renderMode == RenderMode::PERSPECTIVE)
		return glm::perspective(fov, aspectRatio, nearClippingPlane, farClippingPlane);
}
