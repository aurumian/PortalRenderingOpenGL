#pragma once
#include "Actor.h"

class Mesh;
class Material;

class MeshRenderer : public Component{
public:
	MeshRenderer(Mesh* mesh, Material* material) {
		this->mesh = mesh;
		this->material = material;
	}

	void Draw();

	Material* GetMaterial() {
		return material;
	}

private:
	Material* material;
	Mesh* mesh;
};