#include "MeshLoader.h"
#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>


MeshLoader::MeshLoader(std::string filepath)
{
	OpenFile(filepath);
}

MeshLoader::MeshLoader()
{

}


MeshLoader::~MeshLoader()
{
	
}

Mesh MeshLoader::GetMesh(size_t meshIndex)
{
	const aiScene* scene = importer.GetScene();

	vector<Vertex> vertices;
	vector<GLuint> indicies;
	
	aiMesh* mesh = scene->mMeshes[meshIndex];

	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		Vertex v;

		v.position.x = mesh->mVertices[i].x;
		v.position.y = mesh->mVertices[i].y;
		v.position.z = mesh->mVertices[i].z;

		v.normal.x = mesh->mNormals[i].x;
		v.normal.y = mesh->mNormals[i].y;
		v.normal.z = mesh->mNormals[i].z;

		vertices.push_back(v);
	}
	if (mesh->mTextureCoords != nullptr) {
		for (size_t i = 0; i < mesh->mNumVertices; ++i) {
			vertices[i].texCoords.x = mesh->mTextureCoords[0]->x;
			vertices[i].texCoords.y = mesh->mTextureCoords[0]->y;
		}
	}
		

	for (size_t i = 0; i < mesh->mNumFaces; ++i) {
		indicies.push_back(mesh->mFaces[i].mIndices[0]);
		indicies.push_back(mesh->mFaces[i].mIndices[1]);
		indicies.push_back(mesh->mFaces[i].mIndices[2]);
	}

	return Mesh(vertices, indicies);
}

void MeshLoader::OpenFile(std::string filepath)
{
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
	}
}
