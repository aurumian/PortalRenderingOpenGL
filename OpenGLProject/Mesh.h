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
	Mesh(vector<Vertex> vertices, vector<GLuint> indicies, vector<Texture> textures);
	
	void Draw(Shader shader);

	~Mesh();

private:
	GLuint VAO, VBO, EBO;
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	void SetupMesh();
};

