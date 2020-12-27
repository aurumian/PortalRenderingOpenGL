#include "MeshRenderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"

void MeshRenderer::Draw()
{
	Draw(material);
}


void MeshRenderer::Draw(Material* mat) {
	if (!mesh->setUp)
		mesh->SetupMesh();
	if (mat == nullptr)
		mat = material;
	mat->shader->use();

	// set textures
	GLuint diffuseNr = 0;
	GLuint specularNr = 0;
	for (GLuint i = 0; i < mat->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		string number;
		string name = mat->textures[i].type;
		if (name == "texture_diffuse")
			number = to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = to_string(specularNr++);

		mat->shader->setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, mat->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// draw the mesh
	glBindVertexArray(mesh->VAO);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}