#pragma once
#include "Component.h"

class MoverComponent : public Component
{
public:
	MoverComponent();
	~MoverComponent();

	glm::vec3 startPos;
	glm::vec3 endPos;
	float speed = 1.0f;

	virtual void Start() override;
	virtual void Update() override;
private:
	float travelledDist = 0.0f;
	float dir = 1.0f;
};

