#include "SceneGraph.h"

#include <assert.h>

#include "OctNode.h"
#include "Timer.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "Dx11Renderer.h"


SceneGraph::SceneGraph() :
	mNumCreatedObjects(0)
{
	
}

SceneGraph::~SceneGraph()
{

}
#include "AABB.h"//////////////////////////////////////////////////////////////////////////

void SceneGraph::init(unsigned short treeDepth, Dx11Renderer* renderer,
	ShaderManager* shadermanager)
{
	assert(treeDepth != 0 && "Tree depth cannot be zero.");
	assert(renderer);

	mRenderer = renderer;

	//mRootNode = std::make_shared<OctNode>(OctNode(getNumCreatedObjects()));
	//mRootNode = std::shared_ptr<OctNode>(new OctNode(0));
	AABB aabb(100.0f, 100.0f, 100.0f, nullptr);

	mRootNode = std::make_shared<OctNode>(getNumCreatedObjects(), XMFLOAT3(0.0f, 0.0f, 0.0f), 
		this, nullptr, aabb);
	mRootNode->mAABB->mOwner = mRootNode.get();

	mRootNode->createChildren(treeDepth, this);
	mRootNode->prepareForDrawing(mRenderer, shadermanager);

	Log::log("Scene graph initialized successfully");
}

void SceneGraph::drawAABBs(Dx11Renderer* dx11Renderer) const
{
	assert (dx11Renderer);

	/*
	dx11Renderer->getDeviceContext()->VSSetShader(mVertexShader, nullptr, 0);
	dx11Renderer->getDeviceContext()->VSSetConstantBuffers(0, 1, &mCBChangesNever);
	dx11Renderer->getDeviceContext()->VSSetConstantBuffers(1, 1, &mCBChangesOnResize);
	dx11Renderer->getDeviceContext()->VSSetConstantBuffers(2, 1, &mCBChangesEveryFrame);

	dx11Renderer->getDeviceContext()->PSSetShader(mPixelShader, nullptr, 0);
	dx11Renderer->getDeviceContext()->PSSetConstantBuffers(2, 1, &mCBChangesEveryFrame);
	dx11Renderer->getDeviceContext()->PSSetShaderResources(0, 1, &mTextureRV);
	dx11Renderer->getDeviceContext()->PSSetSamplers(0, 1, &mSamplerLinear);
	*/
//	dx11Renderer->getDeviceContext()->DrawIndexed(36, 0, 0);


	mRootNode->drawAABB(dx11Renderer);
}
