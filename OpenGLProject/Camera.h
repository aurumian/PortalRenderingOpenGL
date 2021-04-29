#pragma once

#include "Transform.h"
#include "Physics.h"

// TODO: validate projection matrix values
class Camera
{
protected:
	Transform transform;
	float fov = 45.0f;
	float aspectRatio = 16.0f/9.0f;
	float nearClippingPlane = 3.1f;
	float farClippingPlane = 100.0f;
	glm::mat4 worldToView = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	bool isOrtho = false;

	Pyramid pyramid;
public:
	Camera();
	Camera(float fov, float aspectRatio, float zNear, float zFar);
	Camera(float left, float right, float bottom, float top, float zNear, float zFar);

	~Camera();

	// modifying the returned value won't affect the camera
	// use SetTransform
	Transform GetTransform() const {
		return transform;
	}

	void SetTransform(Transform transform) {
		transform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
		this->transform = transform;
		SetWorldToViewMatrix(transform.GetInverseTransformMatrix()); 
	}

	const glm::mat4& GetWorldToViewMatrix() const;

	const glm::mat4& GetProjectionMatrix() const;

	// assumes the matrix doesn't have a scale
	void SetWorldToViewMatrix(const glm::mat4& worldToView);

	void SetProjectionMatrixOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
	void SetProjectionMatrixPerspective(float fov, float aspectRatio, float zNear, float zFar);

	void SetAspectRatio(float ratio) {
		if (ratio > 0.1)
			aspectRatio = ratio;
		SetProjectionMatrixPerspective(fov, aspectRatio, nearClippingPlane, farClippingPlane);
	}

	bool IsOrtho();

	const Pyramid& GetPyramid() const
	{
		return pyramid;
	}

protected:
	void UpdatePyramid();

};

