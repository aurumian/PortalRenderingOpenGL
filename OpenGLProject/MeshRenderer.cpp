#include "MeshRenderer.h"
#include "Shader.h"
#include "Mesh.h"

void MeshRenderer::Draw()
{
	if (!mesh->setUp)
		mesh->SetupMesh();

	shader->use();

	// set textures
	/*GLuint diffuseNr = 0;
	GLuint specularNr = 0;
	for (GLuint i = 0; i < mesh->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		string number;
		string name = mesh->textures[i].type;
		if (name == "texture_diffuse")
			number = to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = to_string(specularNr++);

		shader->setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);*/

	// draw the mesh
	glBindVertexArray(mesh->VAO);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}