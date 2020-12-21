#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Shader
{
public:
	inline GLuint getProgramID() const{
		return ID;
	}

	Shader(GLuint id) { ID = id; };

	
	
	~Shader() {
		// disabled it cause a parameter passed to function
		// by value also triggers programm deletion
		//glDeleteProgram(ID);
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
	float getFloat(const string &name) {
		float out;
		glGetUniformfv(ID, glGetUniformLocation(ID, name.c_str()), &out);
		return out;
	}
	void setVec3(const string &name, float x, float y, float z) {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec3(const string &name, glm::vec3 vec) {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
	}
	void setVec4(const string& name, glm::vec4 vec) {
		glUniform4f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z, vec.w);
	}
	void setMat4(const string &name, const glm::mat4 &matrix) {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void SetMat3(const string& name, const glm::mat3 &matrix) {
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	}

protected:
	//openGL shader program ID
	GLuint ID;
private:
	static const size_t infoLogSize = 512;
};

