#pragma once

#include "Shader.h"
#include "Transform.h"
#include <vector>

using namespace std;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct Texture {
	GLuint id;
	string type;
};

class Mesh
{
public:
	Mesh(vector<Vertex> vertices, vector<GLuint> indicies);

	~Mesh();

private:
	GLuint VAO, VBO, EBO;
	vector<Vertex> vertices;
	vector<GLuint> indices;

	void SetupMesh();

	bool setUp = false;

	friend class MeshRenderer;
};
