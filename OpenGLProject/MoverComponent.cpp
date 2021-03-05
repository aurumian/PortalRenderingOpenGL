#include "MoverComponent.h"
#include "Actor.h"
#include "Time.h"


MoverComponent::MoverComponent()
{
}


MoverComponent::~MoverComponent()
{
}

void MoverComponent::Start() {
	if (GetOwner() == nullptr)
		return;

	
}


void MoverComponent::Update() {
	if (GetOwner() == nullptr)
		return;

	float dist = (startPos - endPos).length();
	travelledDist += dir * speed * Time::GetDeltaTime();
	float a = travelledDist / dist;
	if (a > 1.0f || a < 0.0f) {
		// reverse directoin
		dir = -dir;
	}
	a = glm::clamp(a, 0.0f, 1.0f);
	GetOwner()->transform.position = startPos * a + endPos * (1 - a);
}