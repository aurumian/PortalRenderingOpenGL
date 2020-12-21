#pragma once
#include "Actor.h"

class Mesh;
class Shader;

class MeshRenderer : public Component{
public:
	MeshRenderer(Mesh* mesh, Shader* shader) {
		this->mesh = mesh;
		this->shader = shader;
	}

	void Draw();

	Shader* GetShader() {
		return shader;
	}

private:
	Shader* shader;
	Mesh* mesh;
};