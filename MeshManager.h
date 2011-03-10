#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include <map>
#include <memory>
#include <string>

#include "Mesh.h"

class Dx11Renderer;
struct aiMesh;


class MeshManager
{
public:
	MeshManager(Dx11Renderer* dx11Renderer);
	~MeshManager();

	const std::shared_ptr<Mesh> getMesh(const std::string& meshName);
	

private:
	std::shared_ptr<Mesh> createMesh(const std::string& meshName);

	void parseData(std::vector<VertexNormalTex>& vertices,
		std::vector<unsigned int>& indices, const aiMesh* mesh);

	//Returns true on success.
	bool createBuffers(const std::vector<VertexNormalTex>& vertices,
		const std::vector<unsigned int>& indices, ID3D11Buffer*& vertexBuffer,
		ID3D11Buffer*& indexBuffer, const std::string& meshName);

	inline unsigned int getNumCreatedMeshes()
	{
		return ++mNumCreatedMeshes;
	}

	std::map<std::string, std::shared_ptr<Mesh>>	mMeshes;

	unsigned int	mNumCreatedMeshes;
	Dx11Renderer*	mDx11Renderer;
};

#endif // MESHMANAGER_H