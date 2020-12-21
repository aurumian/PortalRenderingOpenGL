#include "Material.h"



Material::Material()
{
}

Material::Material(Shader * shader)
{
	this->shader = shader;
}


Material::~Material()
{
}
