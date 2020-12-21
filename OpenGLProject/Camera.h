#pragma once

#include "Transform.h"

enum class RenderMode {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

class Camera
{
protected:
	Transform transform;
	float fov = 45.0f;
	float aspectRatio = 16.0f/9.0f;
	float nearClippingPlane = 0.01f;
	float farClippingPlane = 100.0f;
	RenderMode renderMode = RenderMode::PERSPECTIVE;
public:
	Camera();
	~Camera();

	// modifying the returned value won't affect the camera
	// use SetTransform
	Transform GetTransform() const {
		return transform;
	}

	void SetTransform(Transform transform) {
		this->transform = transform;
	}

	glm::mat4 GetWorldToViewMatrix() const;

	glm::mat4 GetProjectionMatrix() const;

	void SetAspectRatio(float ratio) {
		if (ratio > 0.1)
			aspectRatio = ratio;
	}

};

