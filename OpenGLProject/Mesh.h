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
	Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indicies);

	~Mesh();

	void SetupMesh();

private:
	GLuint VAO, VBO, EBO;
	vector<Vertex> vertices;
	vector<GLuint> indices;

	bool setUp = false;

	friend class MeshRenderer;
};
