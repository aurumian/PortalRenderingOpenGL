#include "MeshRenderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"

void MeshRenderer::Draw()
{
	if (!mesh->setUp)
		mesh->SetupMesh();

	material->shader->use();

	// set textures
	GLuint diffuseNr = 0;
	GLuint specularNr = 0;
	for (GLuint i = 0; i < material->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		string number;
		string name = material->textures[i].type;
		if (name == "texture_diffuse")
			number = to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = to_string(specularNr++);

		material->shader->setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, material->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// draw the mesh
	glBindVertexArray(mesh->VAO);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}