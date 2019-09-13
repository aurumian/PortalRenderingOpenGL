#pragma once
#define GLM_FORCE_LEFT_HANDED
#include <iostream>
#include "Camera.h"
#include <GLFW/glfw3.h>
#include "Time.h"

class FPSCameraController
{
protected:
	Camera* camera;
	// movement speed in meters per second
	float movementSpeed = 10.0f; 
	// look speed at degrees per second
	float lookSpeed = 60.0f;
	float sensetivity = 0.1f;
	float maxPitch = 90.0f;
	float minPitch = -90.0f;
public:
	FPSCameraController(Camera* camera);
	~FPSCameraController();

	void ProcessInput(GLFWwindow* window, float mouseXDelta, float mouseYDelta) {
		float speed = movementSpeed * Time::GetDeltaTime();
		Transform transform = camera->GetTransform();
		glm::vec3 cameraForward = transform.rotation.GetForwardVector();
		glm::vec3 cameraRight = transform.rotation.GetRightVector();

		

		// movement control
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			transform.position += cameraForward * speed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			transform.position -= cameraForward * speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			transform.position -= cameraRight * speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			transform.position += cameraRight * speed;

		

		// look around control
		glm::vec3 euler = transform.rotation.GetEulerAngles();
		euler.x -= mouseYDelta * sensetivity;
		if (euler.x > maxPitch)
			euler.x = maxPitch;
		else if (euler.x < minPitch)
			euler.x = minPitch;
		euler.y += mouseXDelta * sensetivity;
		transform.rotation.SetEulerAngles(euler);

		camera->SetTransform(transform);
	}

	Camera* GetCamera() {
		return camera;
	}
};

