#pragma once
#include <vector>

class Shader;
class MeshRenderer;
class Texture;

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
	~Material();

	FaceCulling culling = FaceCulling::NO_CULL;

	Shader* GetShader() { return shader; }

private:
	Shader* shader;
	std::vector<Texture*> textures;
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