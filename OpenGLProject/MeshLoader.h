#pragma once

#include <string>

class Mesh;

#include <assimp/Importer.hpp>

class MeshLoader
{
public:
	MeshLoader(std::string filepath);
	MeshLoader();
	~MeshLoader();

	Mesh GetMesh(size_t meshIndex);

	void OpenFile(std::string filepath);

private:
	

	Assimp::Importer importer;
};

