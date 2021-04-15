#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class ShaderProgram
{
public:
	ShaderProgram(const char * vertexPath, const char* fragmentPath)
	{
		//1.get data
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		// ensure ifstream objects can throw exceptions: 
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		// read data
		try
		{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR:::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl; 
			std::cout << e.what() << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		//2.compile shaders
		unsigned int vertex, fragment;

		//vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		//print errors, if any
		CheckShaderCompileErrors(vertex);

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		//print errors, if any
		CheckShaderCompileErrors(fragment);

		//3. Creating Shader Program
		Program = glCreateProgram();
		glAttachShader(Program, vertex);
		glAttachShader(Program, fragment);
		glLinkProgram(Program);
		//Print linking errors if any
		int success;
		char infolog[512];
		glGetProgramiv(Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(Program, 512, NULL, infolog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
		}

		// delete the shaders as they’re linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void Use() {
		glUseProgram(Program);
	}

	/*Set of methods to set uniform values*/
	void setUniformValue(const std::string UniformName, bool Value)
	{
		glUniform1i(glGetUniformLocation(Program, UniformName.c_str()), (int)Value);
	}
	void setUniformValue(const std::string UniformName, int Value) 
	{
		glUniform1i(glGetUniformLocation(Program, UniformName.c_str()), Value);
	}
	void setUniformValue(const std::string UniformName, float Value)
	{
		glUniform1f(glGetUniformLocation(Program, UniformName.c_str()), Value);
	}

private:
	unsigned int Program;

	//prints shader compile errors if any
	void CheckShaderCompileErrors(int Shader)
	{
		int success;
		char infolog[512];

		glGetShaderiv(Shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(Shader, 512, NULL, infolog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infolog << std::endl;
		}
	}
};