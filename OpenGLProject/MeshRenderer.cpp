#include "MeshRenderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"

void MeshRenderer::Draw()
{
	Draw(material);
}


void MeshRenderer::Draw(const Material* mat) {
	if (!mesh->setUp)
		mesh->SetupMesh();


	if (mat == nullptr)
		mat = material;
	mat->shader->Use();

	mat->shader->setMat4("objectToWorld", GetTransform().GetTransformMatrix());

	// set textures
	GLuint diffuseNr = 0;
	GLuint specularNr = 0;
	for (GLuint i = 0; i < material->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i + 8 /* offset for shadowmaps */);
		string number;
		string name = material->textures[i].type;
		if (name == "texture_diffuse")
			number = to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = to_string(specularNr++);

		mat->shader->setInt(("material." + name + number).c_str(), i + 8);
		glBindTexture(GL_TEXTURE_2D, material->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// draw the mesh
	glBindVertexArray(mesh->VAO);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}