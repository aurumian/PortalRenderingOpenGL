#pragma once
#include <vector>
#include "Mesh.h"

class Shader;
class MeshRenderer;


 enum FaceCulling {
	NO_CULL,
	CULL_BACK,
	CULL_FRONT,
};
class Material
{
public:
	Material();
	Material(Shader* shader);
	Material(Shader* shader, std::vector<Texture> textures);
	~Material();

	FaceCulling culling = FaceCulling::NO_CULL;

	Shader* GetShader() { return shader; }

	std::vector<Texture> textures;
	Shader* shader;

private:
	
	
	friend class MeshRenderer;
};



class MaterialProperty {
public:
	virtual void Apply() = 0;
	virtual ~MaterialProperty() {}

	MaterialProperty() {}

	std::string name;
protected:
	Material* material;
	friend class Material;
};