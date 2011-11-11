#pragma once

#include <vector>
#include <numeric>

#include <Windows.h>
#include <d3d11.h>

#include "bsCollision.h"

class bsDx11Renderer;
class bsEntity;


/*	Class containing a single mesh.
	Contains vertex and index buffers, and can render itself.
*/
__declspec(align(16)) class bsMesh
{
public:
	inline void* operator new(size_t)
	{
		return _aligned_malloc(sizeof(bsMesh), 16);
	}

	inline void operator delete(void* p)
	{
		_aligned_free(p);
	}


	//For container purposes only, do not use this constructor.
	inline bsMesh(unsigned int id)
		: mID(id)
		, mLoadingFinished(false)
	{}

	/*	Creates a mesh given a unique ID, vertex and index buffer(s), index count for each
		index/vertex buffer pair, and an AABB whose extents covers every single vertex
		in the vertex buffer.
	*/
	bsMesh(unsigned int id, std::vector<ID3D11Buffer*>&& vertexBuffers,
		std::vector<ID3D11Buffer*>&& indexBuffers,
		std::vector<unsigned int>&& indexCounts, std::vector<unsigned int>&& vertexCounts,
		const bsCollision::Sphere& boundingSphere);

	~bsMesh();

	bsMesh& operator=(bsMesh&& other);

	/*	Renders the mesh.
		Calling this function before the mesh has finished loading results in undefined
		behavior.
	*/
	void draw(bsDx11Renderer* dx11Renderer) const;

	void drawInstanced(ID3D11DeviceContext& deviceContext, ID3D11Buffer* instanceBuffer,
		unsigned int instanceCount) const;

	/*	Returns true if this mesh has finished loading and is ready to be rendered.
	*/
	inline bool hasFinishedLoading() const
	{
		return mLoadingFinished;
	}

	void attachedToEntity(bsEntity& entity);

	/*	Returns the total amount of triangles in this mesh.
	*/
	inline unsigned int getTriangleCount() const
	{
		return (unsigned int)std::accumulate(std::begin(mVertexCounts),
			std::end(mVertexCounts), 0) / 3;
	}


	inline const std::vector<ID3D11Buffer*>& getVertexBuffers() const
	{
		return mVertexBuffers;
	}

	inline const std::vector<ID3D11Buffer*>& getIndexBuffers() const
	{
		return mIndexBuffers;
	}

	inline const std::vector<unsigned int>& getIndexCounts() const
	{
		return mIndexCounts;
	}

	inline const bsCollision::Sphere& getBoundingSphere() const
	{
		return mBoundingSphere;
	}

private:
	//Not copyable
	bsMesh(const bsMesh&);
	void operator=(const bsMesh&);

	bsCollision::Sphere mBoundingSphere;

	std::vector<ID3D11Buffer*>	mVertexBuffers;
	std::vector<ID3D11Buffer*>	mIndexBuffers;
	std::vector<unsigned int>	mIndexCounts;
	std::vector<unsigned int>	mVertexCounts;

	unsigned int	mID;

	//0 if loading is not finished, positive value otherwise.
	volatile unsigned int	mLoadingFinished;

	std::vector<bsEntity*>	mEntities;
};
