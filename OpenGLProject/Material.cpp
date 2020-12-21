#include "Material.h"



Material::Material()
{
}

Material::Material(Shader * shader)
{
	this->shader = shader;
}

Material::Material(Shader * shader, std::vector<Texture> textures)
{
	this->shader = shader;
	this->textures = textures;
}


Material::~Material()
{
}
