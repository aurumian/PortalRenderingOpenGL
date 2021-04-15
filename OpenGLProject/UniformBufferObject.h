#pragma once

#include <glad/glad.h>
#include <string>

class Shader;

class UniformBufferObject
{
public:
	UniformBufferObject(const std::string& name, GLsizeiptr size);

	std::string GetName();

	void BindToShader(Shader* shader);

	void SetBufferSubData(GLintptr offset, GLsizeiptr size, const void* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, id);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	~UniformBufferObject();

protected:
	GLuint id;
	std::string name;
	GLuint bindingPoint;
	static GLuint nextBindingPoint;
};

