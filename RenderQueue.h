#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H

#include <vector>
#include <memory>

#include "Dx11Renderer.h"

class SceneNode;
class Renderable;
class Mesh;
class Primitive;
class Dx11Renderer;
class ShaderManager;
class VertexShader;
class PixelShader;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
//struct XMFLOAT4X4;


class RenderQueue
{
public:
	RenderQueue(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager);
	~RenderQueue();

	//Clears all current collections of renderables.
	void reset();

	/*	Add renderables to the queue. The objects must never expire before the current
		frame has completed rendering
	*/
	void addRenderables(const std::vector<std::shared_ptr<Renderable>>& renderables);

	void draw(Dx11Renderer* dx11Renderer);

	void addSceneNode(SceneNode* sceneNode);

private:
	void sortSceneNodes();

	//Sorts the renderables into their respective collections.
	void splitRenderables();

	void sortMeshes();

	void sortPrimitives();

	void drawMesh(Mesh* mesh, ID3D11DeviceContext* deviceContext);

	void setWorldConstantBuffer(const XMFLOAT4X4& world);

	//null to unbind
	//Sets both vertex shader and input layout.
	void setVS(VertexShader* vs);

	//null to unbind
	void setPS(PixelShader* ps);

	//Unbinds only.
	void setGS(void* gs);

	std::vector<std::shared_ptr<Renderable>>	mRenderables;

	std::vector<Mesh*>		mMeshes;
	std::vector<Primitive*>	mWireframePrimitives;
	std::vector<SceneNode*>	mSceneNodes;
	std::vector<std::pair<SceneNode*, Renderable*>>	mRenderablePairs;

	Dx11Renderer*	mDx11Renderer;
	ShaderManager*	mShaderManager;
	ID3D11Buffer*	mWorldBuffer;
	bool			mWorldBufferSet;
};

#endif // RENDERQUEUE_H