#include "UniformBufferObject.h"

#include "Shader.h"


GLuint UniformBufferObject::nextBindingPoint = 0;

UniformBufferObject::UniformBufferObject(const std::string& name, GLsizeiptr size)
{
	this->name = name;
	this->bindingPoint = nextBindingPoint++;
	glGenBuffers(1, &id);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, id);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBufferObject::~UniformBufferObject()
{
	glDeleteBuffers(1, &id);
}

std::string UniformBufferObject::GetName() {
	return name;
}




void UniformBufferObject::BindToShader(Shader* shader)
{
	GLuint uboIndex = glGetUniformBlockIndex(shader->GetProgramID(), name.c_str());
	glUniformBlockBinding(shader->GetProgramID(), uboIndex, bindingPoint);
}