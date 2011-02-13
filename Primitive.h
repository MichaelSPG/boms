#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "Dx11Renderer.h"

class ShaderManager;
class VertexShader;
class AABB;
class PixelShader;
class VertexShader;

struct Vertex
{
	XMFLOAT3 Pos;
};

class Primitive
{
	friend class OctNode;

public:
	Primitive();
	~Primitive();

	void createPrimitive(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager,
		const AABB& aabb);

	void draw(Dx11Renderer* dx11Renderer);
	
private:
	std::shared_ptr<VertexShader>	mVertexShader;
	std::shared_ptr<PixelShader>	mPixelShader;

	ID3D11Buffer*		mBuffer;

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
};

#endif // PRIMITIVE_H