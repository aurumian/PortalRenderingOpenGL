#pragma once

#include "Actor.h"
#include "Camera.h"

class Player : public Actor
{
public:
	Player();
	~Player();

	typedef void (*OnTriggerHit)(const RayHit& hit);

	OnTriggerHit onTriggerHitCallback;

	Player(Camera* camera) {
		this->camera = camera;
	}

	Camera* camera;

	virtual void OnRayHit(RayHit hit) override;

	void ProcessInput(glm::vec2 movementInput, glm::vec2 camInput);

	void SetCameraTransfrom(Transform t);

protected:
	// movement speed in meters per second
	float movementSpeed = 10.0f;
	// look speed at degrees per second
	float lookSpeed = 60.0f;
	float sensetivity = 0.1f;
	float maxPitch = 80.0f;
	float minPitch = -80.0f;
};

