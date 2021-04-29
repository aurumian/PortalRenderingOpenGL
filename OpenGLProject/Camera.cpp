#include "Camera.h"

Camera::Camera()
{
	SetProjectionMatrixPerspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
}

Camera::Camera(float fov, float aspectRatio, float zNear, float zFar)
{
	SetProjectionMatrixPerspective(fov, aspectRatio, zNear, zFar);
}

Camera::Camera(float left, float right, float bottom, float top, float zNear, float zFar)
{
	SetProjectionMatrixOrtho(left, right, bottom, top, zNear, zFar);
}


Camera::~Camera()
{
}

const glm::mat4& Camera::GetWorldToViewMatrix() const
{
	return worldToView;
}

const glm::mat4& Camera::GetProjectionMatrix() const
{
	return projection;
}

void Camera::SetWorldToViewMatrix(const glm::mat4& worldToView)
{
	glm::mat4 inverse = glm::inverse(worldToView);
	transform.position = inverse * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	transform.rotation = Rotator(glm::quat_cast(glm::mat3(inverse)));
	if (!IsOrtho())
		UpdatePyramid();
	this->worldToView = worldToView;
}

void Camera::SetProjectionMatrixOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	projection = glm::ortho(left, right, bottom, top, zNear, zFar);
	isOrtho = true;
}

void Camera::SetProjectionMatrixPerspective(float fov, float aspectRatio, float zNear, float zFar)
{
	this->fov = fov;
	this->aspectRatio = aspectRatio; 
	this->nearClippingPlane = zNear;
	this->farClippingPlane = zFar;
	projection = glm::perspective(fov, aspectRatio, zNear, zFar);
	UpdatePyramid();
	isOrtho = false;
}

bool Camera::IsOrtho()
{
	return isOrtho;
}

void Camera::UpdatePyramid()
{
	glm::mat4 t = transform.GetTransformMatrix();
	glm::vec3 apex = t * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 points[4];
	float y = tan(fov / 2.0f) * farClippingPlane;
	float x = y * aspectRatio;
	points[0] = glm::vec3(-x, -y, farClippingPlane);
	points[1] = glm::vec3(-x, +y, farClippingPlane);
	points[2] = glm::vec3(+x, +y, farClippingPlane);
	points[3] = glm::vec3(+x, -y, farClippingPlane);
	for (size_t i = 0; i < 4; ++i)
	{
		points[i] = t * glm::vec4(points[i], 1.0f);
	}
	pyramid.Construct(points, apex);
}
