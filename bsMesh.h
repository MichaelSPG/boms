#pragma once

#include "bsConfig.h"

#include <string>
#include <vector>
#include <memory>

#include <Windows.h>
#include <d3d11.h>
#include <D3DX11.h>

#include <Common/Base/hkBase.h>

#include "bsRenderable.h"

class bsDx11Renderer;


/*	Class containing a single mesh.
	Contains vertex and index buffers, and can render itself.
*/
class bsMesh : public bsRenderable
{
public:
	//For container purposes only, do not use this constructor.
	inline bsMesh()
		: mLoadingFinished(false)
	{}

	//Initialize a mesh with data created from bsMeshCreator.
	bsMesh(unsigned int id, std::vector<ID3D11Buffer*>&& vertexBuffers,
		std::vector<ID3D11Buffer*>&& indexBuffers,
		std::vector<unsigned int>&& indices);

	~bsMesh();

	bsMesh& operator=(bsMesh&& other);

	/*	Renders the mesh.
		Calling this function before the mesh has finished loading results in undefined
		behavior.
	*/
	void draw(bsDx11Renderer* dx11Renderer) const;

	inline RenderableType getRenderableType() const
	{
		return MESH;
	}

	/*	Returns true if this mesh has finished loading and is ready to be rendered.
	*/
	inline bool hasFinishedLoading() const
	{
		return mLoadingFinished;
	}

	//Set min and max point of this mesh' AABB.
	void setAabb(const hkVector4& min, const hkVector4& max);


private:
	//Not copyable
	bsMesh(const bsMesh&);
	void operator=(const bsMesh&);

	std::vector<ID3D11Buffer*>	mVertexBuffers;
	std::vector<ID3D11Buffer*>	mIndexBuffers;
	std::vector<unsigned int>	mIndexCounts;

	unsigned int	mID;

	bool			mLoadingFinished;
};
