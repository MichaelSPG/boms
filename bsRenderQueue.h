#ifndef BS_RENDERQUEUE_H
#define BS_RENDERQUEUE_H

#include <vector>
#include <memory>

#include "bsDx11Renderer.h"

class bsSceneNode;
class bsRenderable;
class bsMesh;
class bsPrimitive;
class bsDx11Renderer;
class bsShaderManager;
class bsVertexShader;
class bsPixelShader;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
//struct XMFLOAT4X4;


class bsRenderQueue
{
public:
	bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager);
	~bsRenderQueue();

	//Clears all current collections of renderables.
	void reset();

	/*	Add renderables to the queue. The objects must never expire before the current
		frame has completed rendering
	*/
	void addRenderables(const std::vector<std::shared_ptr<bsRenderable>>& renderables);

	void draw(bsDx11Renderer* dx11Renderer);

	void addSceneNode(bsSceneNode* sceneNode);

private:
	void sortSceneNodes();

	//Sorts the renderables into their respective collections.
	void splitRenderables();

	void sortMeshes();

	void sortPrimitives();

	void drawMesh(bsMesh* mesh, ID3D11DeviceContext* deviceContext);

	void setWorldConstantBuffer(const XMFLOAT4X4& world);

	//null to unbind
	//Sets both vertex shader and input layout.
	void setVS(bsVertexShader* vs);

	//null to unbind
	void setPS(bsPixelShader* ps);

	//Unbinds only.
	void setGS(void* gs);

	std::vector<std::shared_ptr<bsRenderable>>	mRenderables;

	std::vector<bsMesh*>		mMeshes;
	std::vector<bsPrimitive*>	mWireframePrimitives;
	std::vector<bsSceneNode*>	mSceneNodes;
	std::vector<std::pair<bsSceneNode*, bsRenderable*>>	mRenderablePairs;

	bsDx11Renderer*	mDx11Renderer;
	bsShaderManager*	mShaderManager;
	ID3D11Buffer*	mWorldBuffer;
	bool			mWorldBufferSet;
};

#endif // BS_RENDERQUEUE_H
