#include "bsDeferredRenderer.h"

#include <cassert>
#include <vector>
#include <string>

#include <d3d11.h>

#include "bsDx11Renderer.h"
#include "bsCamera.h"
#include "bsRenderQueue.h"
#include "bsRenderTarget.h"
#include "bsWindow.h"
#include "bsFullScreenQuad.h"
#include "bsShaderManager.h"


bsDeferredRenderer::bsDeferredRenderer(bsDx11Renderer* dx11Renderer, bsCamera* camera,
	bsShaderManager* shaderManager, bsWindow* window)
	: mDx11Renderer(dx11Renderer)
	, mCamera(camera)
	, mShaderManager(shaderManager)
{
	assert(dx11Renderer);
	assert(camera);
	assert(shaderManager);

	mRenderQueue = new bsRenderQueue(mDx11Renderer, mShaderManager);
	mRenderQueue->setCamera(mCamera);

	ID3D11Device* device = mDx11Renderer->getDevice();
	const int windowWidth = window->getWindowWidth(), windowHeight = window->getWindowHeight();
	
	mGBuffer.position	= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.normal		= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.diffuse	= new bsRenderTarget(windowWidth, windowHeight, device);

#if BS_DEBUG_LEVEL > 0
	//Set names for debugging purposes
	//Position
	std::string debugData = "GBuffer Position SRV";
	mGBuffer.position->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "GBuffer Position RTV";
	mGBuffer.position->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Normal
	debugData = "GBuffer Normal SRV";
	mGBuffer.normal->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "GBuffer Normal RTV";
	mGBuffer.normal->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Diffuse
	debugData = "GBuffer Diffuse SRV";
	mGBuffer.diffuse->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "GBuffer Diffuse RTV";
	mGBuffer.diffuse->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());
#endif

	mFullScreenQuad = new bsFullScreenQuad(device);

	createShaders();
}

bsDeferredRenderer::~bsDeferredRenderer()
{
	delete mGBuffer.diffuse;
	delete mGBuffer.normal;
	delete mGBuffer.position;
}

void bsDeferredRenderer::createShaders()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;

	D3D11_INPUT_ELEMENT_DESC inputDesc;
	memset(&inputDesc, 0, sizeof(inputDesc));
	inputDesc.SemanticName = "POSITION";
	inputDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout.push_back(inputDesc);

	inputDesc.SemanticName = "TEXCOORD";
	inputDesc.AlignedByteOffset = 12;
	inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(inputDesc);

	mMergerVertexShader = mShaderManager->getVertexShader("Merger.fx", inputLayout);
	mMergerPixelShader = mShaderManager->getPixelShader("Merger.fx");
}

void bsDeferredRenderer::renderOneFrame()
{
	mDx11Renderer->clearBackBuffer();

	//Set and clear G buffer
	mDx11Renderer->setRenderTargets(&mGBuffer.position, 3);
	mDx11Renderer->clearRenderTargets(&mGBuffer.position, 3, nullptr);

	//Draw the geometry into the set G buffers.
	mRenderQueue->draw();

	//Unbind render targets.
	mDx11Renderer->setRenderTargets(nullptr, 3);

	mShaderManager->setPixelShader(mMergerPixelShader);
	mShaderManager->setVertexShader(mMergerVertexShader);

	mDx11Renderer->setBackBufferAsRenderTarget();
	
	//Set the GBuffer as shader resources, allowing the merger shader to sample them.
	ID3D11ShaderResourceView* shaderResourceViews[3];
	shaderResourceViews[0] = mGBuffer.position->getShaderResourceView();
	shaderResourceViews[1] = mGBuffer.normal->getShaderResourceView();
	shaderResourceViews[2] = mGBuffer.diffuse->getShaderResourceView();
	mDx11Renderer->getDeviceContext()->PSSetShaderResources(0, 3, shaderResourceViews);

	//Draw a fullscreen quad with the merger shader to produce final output.
	mFullScreenQuad->draw(mDx11Renderer->getDeviceContext());

	//Unbind shader resource views.
	//ID3D11ShaderResourceView* dummy[3] = { nullptr, nullptr, nullptr };
	memset(shaderResourceViews, 0, sizeof(ID3D11ShaderResourceView*)
		* ARRAYSIZE(shaderResourceViews));
	mDx11Renderer->getDeviceContext()->PSSetShaderResources(0, 3, shaderResourceViews);

	//Call the callbacks
	for (unsigned int i = 0, count = mEndOfRenderCallbacks.size(); i < count; ++i)
	{
		mEndOfRenderCallbacks[i]();
	}

	mDx11Renderer->present();
}
