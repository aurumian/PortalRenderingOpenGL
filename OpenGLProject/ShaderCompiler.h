#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>

class Shader;

class ShaderCompiler
{
public:
	ShaderCompiler();
	~ShaderCompiler();

	void SetVertexShader(const GLchar* filepath);
	void SetFragmentShader(const GLchar* filepath);

	Shader* Compile() const;

	static std::vector<Shader*> GetAllShaders() {
		return shaderDB;
	}

private:
	static const size_t infoLogSize = 512;
	static std::vector<Shader*> shaderDB;
	std::string vertexPath;
	std::string fragPath;
	bool CheckShaderCompileErrors(GLuint shaderID, std::string firstLine = "ERROR::SHADER::COMPILATION_FAILED") const;
};

