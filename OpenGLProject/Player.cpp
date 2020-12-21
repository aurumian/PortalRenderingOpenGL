#include "Player.h"
#include "Time.h"
#include "Physics.h"


Player::Player()
{
}


Player::~Player()
{
}

void Player::OnRayHit(RayHit hit){
	
}

void Player::ProcessInput(glm::vec2 movementInput, glm::vec2 mouseInput)
{
	if (camera == nullptr) return;

	float speed = movementSpeed * Time::GetDeltaTime();
	Transform transform = camera->GetTransform();
	glm::vec3 cameraForward = transform.rotation.GetForwardVector();
	glm::vec3 cameraRight = transform.rotation.GetRightVector();

	// movement control 
	glm::vec3 deltaPos(0.0f, 0.0f, 0.0f);
	deltaPos = (movementInput.y * cameraForward + movementInput.x * cameraRight) * speed;
	

	Ray ray;
	ray.direction = glm::normalize(deltaPos);
	ray.maxDistance = glm::length(deltaPos);
	ray.origin = transform.position;
	RayHit hit = Physics::Instance()->RayCast(ray);
	

	transform.position += deltaPos;

	glm::vec3 deltaEuler = { 0.0f, 0.0f, 0.0f };
	deltaEuler.x = mouseInput.y * sensetivity;
	deltaEuler.y = (mouseInput.x * sensetivity);

	transform.rotation.RotateArounAxis(deltaEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
	transform.rotation.RotateArounAxis(deltaEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
	
	SetCameraTransfrom(transform);

	// if hit try going through a portal only after updating the values
	if (hit.hitSmth && hit.collider->isTrigger) {
		if (onTriggerHitCallback != nullptr)
			onTriggerHitCallback(hit);
	}
}

void Player::SetCameraTransfrom(Transform t) {
	glm::vec3 euler(0.0f, 0.0f, 0.0f);
	// get rotation direction vector
	glm::vec3 dir = t.rotation.GetQuaterion() * glm::vec3(0.0f, 0.0f, 1.0f);
	euler.x = -glm::asin(dir.y);
	float a = dir.x / (glm::length(glm::vec2(dir.x, dir.z)));
	if (dir.z >= 0.0f)
		euler.y = glm::asin(a);
	else
		euler.y = glm::pi<float>() - glm::asin(a);
	euler = euler * 180.0f / glm::pi<float>();

	if (euler.x > maxPitch)
		euler.x = maxPitch;
	else if (euler.x < minPitch)
		euler.x = minPitch;

	t.rotation = euler;

	camera->SetTransform(t);
}
