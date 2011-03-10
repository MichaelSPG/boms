#ifndef CUBE_H
#define CUBE_H

#include "Dx11Renderer.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "MeshManager.h"
#include "VertexShader.h"
#include "PixelShader.h"


struct VSin
{
	XMFLOAT3 position;
	XMFLOAT3 color;
};

class Cube
{
public:
	Cube() {}
	~Cube() {}
	void create(Dx11Renderer* renderer, ShaderManager* shaderManager, MeshManager* meshManager);

	void draw(Dx11Renderer* renderer);
	

private:
	std::shared_ptr<VertexShader>	mVertexShader;
	std::shared_ptr<PixelShader>	mPixelShader;

	std::shared_ptr<Mesh>			mMesh;

	ID3D11Buffer*		mEveryFrame;

	XMFLOAT4X4			mWorld;
};

#endif // CUBE_H