#ifndef CUBE_H
#define CUBE_H

#include "bsDx11Renderer.h"
#include "bsShaderManager.h"
#include "bsResourceManager.h"
#include "bsMeshManager.h"
#include "bsVertexShader.h"
#include "bsPixelShader.h"


struct VSin
{
	XMFLOAT3 position;
	XMFLOAT3 color;
};

class bsCube
{
public:
	bsCube() {}
	~bsCube() {}
	void create(bsDx11Renderer* renderer, bsShaderManager* shaderManager, bsMeshManager* meshManager);

	void draw(bsDx11Renderer* renderer);
	

private:
	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mPixelShader;

	std::shared_ptr<bsMesh>			mMesh;

	ID3D11Buffer*		mEveryFrame;

	XMFLOAT4X4			mWorld;
};

#endif // CUBE_H