#pragma once
class Time
{
protected:
	static float lastFrameTime;
	static float deltaTime;
	Time();
public:

	static  void Init();

	static void Update();

	static float GetDeltaTime() {
		return deltaTime;
	}



};

