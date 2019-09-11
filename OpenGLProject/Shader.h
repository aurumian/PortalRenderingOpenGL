#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class Shader
{
public:
	inline GLuint getProgramID() const{
		return ID;
	}

	// reads and builds the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
		// get source code from files
		string vertexSource;
		string fragmentSource;
		ifstream vShaderFile;
		ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try {
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			// read files' buffer contents into streams
			stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close files
			vShaderFile.close();
			fShaderFile.close();
			// get strings from streams
			vertexSource = vShaderStream.str();
			fragmentSource = fShaderStream.str();
		}
		catch (ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
			cout << e.what() << endl;
		}
		// compile shaders
		GLuint vertex, fragment;
		// vertex
		const char* vSource = vertexSource.c_str();
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vSource, NULL);
		glCompileShader(vertex);
		checkShaderCompileErrors(vertex, "ERROR::SHADER::VERTEX::COMPILATION_FATILED");
		//fragment
		const char* fSource = fragmentSource.c_str();
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fSource, NULL);
		glCompileShader(fragment);
		checkShaderCompileErrors(fragment, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED");

		// create and link program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[infoLogSize];
			glGetProgramInfoLog(ID, infoLogSize, NULL, infoLog);
			cout << "ERROR::SHADER::LINKING_FAILED" << infoLog << endl;
		}

		// clean up
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	
	~Shader() {
		glDeleteProgram(ID);
	}

	//use/activate shader
	void use() {
		glUseProgram(ID);
	}

	// utility uniform functions
	void setBool(const string &name, bool value) {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const string &name, int value) {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const string &name, float value) {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec3(const string &name, float x, float y, float z) {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

protected:
	//openGL shader program ID
	GLuint ID;
private:
	static const size_t infoLogSize = 512;
	void checkShaderCompileErrors(GLuint shaderID, string firstLine = "ERROR::SHADER::COMPILATION_FAILED") {
		firstLine += '\n';
		GLint success;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[infoLogSize];
			glGetShaderInfoLog(shaderID, infoLogSize, NULL, infoLog);
			cout << firstLine << infoLog << endl;
		}
	}
};

