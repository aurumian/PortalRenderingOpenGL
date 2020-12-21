#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Time.h"
#include "Player.h"

class PlayerController
{
protected:
	Player* player;
	// movement speed in meters per second
	float movementSpeed = 10.0f; 
	// look speed at degrees per second
	float lookSpeed = 60.0f;
	float sensetivity = 0.1f;
	float maxPitch = 90.0f;
	float minPitch = -90.0f;

	glm::vec3 euler = glm::vec3(0.0f, 0.0f, 0.0f);
public:
	PlayerController(Player* player);
	~PlayerController();

	void ProcessInput(GLFWwindow* window, float mouseXDelta, float mouseYDelta) {

		glm::vec2 movementInput(0.0f, 0.0f);

		// movement control 
		glm::vec3 deltaPos(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			movementInput.y += 1.0f;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			movementInput.y -= 1.0f;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			movementInput.x -= 1.0f;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			movementInput.x += 1.0f;
		if (glm::length(movementInput) > 0.0f)
			movementInput = glm::normalize(movementInput);
		player->ProcessInput(movementInput, glm::vec2(mouseXDelta, mouseYDelta));

	}

	Player* GetPlayer() {
		return player;
	}
};

