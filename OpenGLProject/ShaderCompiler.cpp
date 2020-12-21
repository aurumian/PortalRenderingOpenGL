#include "ShaderCompiler.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.h"

using namespace std;

std::vector<Shader*> ShaderCompiler::shaderDB = std::vector<Shader*>();

ShaderCompiler::ShaderCompiler()
{
}


ShaderCompiler::~ShaderCompiler()
{
}

void ShaderCompiler::AddVertexShader(const GLchar * filepath)
{
	vertexPath = filepath;
}

void ShaderCompiler::AddFragmentShader(const GLchar * filepath)
{
	fragPath = filepath;
}

Shader * ShaderCompiler::Compile() const
{
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
		fShaderFile.open(fragPath);
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
	if (!CheckShaderCompileErrors(vertex, "ERROR::SHADER::VERTEX::COMPILATION_FATILED")) {
		return nullptr;
	}
	//fragment
	const char* fSource = fragmentSource.c_str();
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fSource, NULL);
	glCompileShader(fragment);
	if (!CheckShaderCompileErrors(fragment, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED")) {
		return nullptr;
	}
	
	// create and link program
	GLuint ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	GLint success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[infoLogSize];
		glGetProgramInfoLog(ID, infoLogSize, NULL, infoLog);
		cout << "ERROR::SHADER::LINKING_FAILED" << infoLog << endl;
		return nullptr;
	}

	// clean up
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	Shader* sh = new Shader(ID);
	shaderDB.push_back(sh);

	return sh;
}

bool ShaderCompiler::CheckShaderCompileErrors(GLuint shaderID, string firstLine) const {
	firstLine += '\n';
	GLint success;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[infoLogSize];
		glGetShaderInfoLog(shaderID, infoLogSize, NULL, infoLog);
		cout << firstLine << infoLog << endl;
	}
	return success;
}
