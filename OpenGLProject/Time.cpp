#include "Time.h"

#include <GLFW/glfw3.h>

float Time::deltaTime;
float Time::lastFrameTime;

Time::Time() {

}
void Time::Init()
{
	lastFrameTime = glfwGetTime();
}
void Time::Update()
{
	float currentTime = glfwGetTime();
	deltaTime = currentTime - lastFrameTime;
	lastFrameTime = currentTime;
}
