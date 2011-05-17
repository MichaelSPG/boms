#include "bsDeferredRenderer.h"

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
#include "bsAssert.h"


bsDeferredRenderer::bsDeferredRenderer(bsDx11Renderer* dx11Renderer, bsCamera* camera,
	bsShaderManager* shaderManager, bsWindow* window)
	: mDx11Renderer(dx11Renderer)
	, mCamera(camera)
	, mShaderManager(shaderManager)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(camera);
	BS_ASSERT(shaderManager);

	mRenderQueue = new bsRenderQueue(mDx11Renderer, mShaderManager);
	mRenderQueue->setCamera(mCamera);

	ID3D11Device* device = mDx11Renderer->getDevice();
	const int windowWidth = window->getWindowWidth(), windowHeight = window->getWindowHeight();
	
	mGBuffer.position	= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.normal		= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.diffuse	= new bsRenderTarget(windowWidth, windowHeight, device);
	mLightRenderTarget	= new bsRenderTarget(windowWidth, windowHeight, device);

#if BS_DEBUG_LEVEL > 0
	//Set names for debugging purposes
	//Position
	std::string debugData = "SRV GBuffer Position";
	mGBuffer.position->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTV GBuffer Position";
	mGBuffer.position->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTT GBuffer Position";
	mGBuffer.position->getRenderTargetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Normal
	debugData = "SRV GBuffer Normal";
	mGBuffer.normal->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTV GBuffer Normal";
	mGBuffer.normal->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTT GBuffer Normal";
	mGBuffer.normal->getRenderTargetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Diffuse
	debugData = "SRV GBuffer Diffuse";
	mGBuffer.diffuse->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTV GBuffer Diffuse";
	mGBuffer.diffuse->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTT GBuffer Diffuse";
	mGBuffer.diffuse->getRenderTargetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Light
	debugData = "SRV Light";
	mLightRenderTarget->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTV Light";
	mLightRenderTarget->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTT Light";
	mLightRenderTarget->getRenderTargetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());
#endif

	mFullScreenQuad = new bsFullScreenQuad(device);

	createShaders();
}

bsDeferredRenderer::~bsDeferredRenderer()
{
	delete mFullScreenQuad;

	delete mLightRenderTarget;
	delete mGBuffer.diffuse;
	delete mGBuffer.normal;
	delete mGBuffer.position;

	delete mRenderQueue;
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
	mRenderQueue->reset();


	//Set and clear G buffer
	mDx11Renderer->setRenderTargets(&mGBuffer.position, 3);

	//Draw the geometry into the G buffers.
	mRenderQueue->drawGeometry();

	//Unbind G buffer render targets.
	mDx11Renderer->setRenderTargets(nullptr, 3);

	//Set the GBuffer as shader resources, allowing them to be used as input by shaders
	ID3D11ShaderResourceView* shaderResourceViews[4];
	shaderResourceViews[0] = mGBuffer.position->getShaderResourceView();
	shaderResourceViews[1] = mGBuffer.normal->getShaderResourceView();
	shaderResourceViews[2] = mGBuffer.diffuse->getShaderResourceView();
	mDx11Renderer->getDeviceContext()->PSSetShaderResources(0, 3, shaderResourceViews);

	//Set the light render target.
	mDx11Renderer->setRenderTargets(&mLightRenderTarget, 1);

	//Draw lights
	//mDx11Renderer->clearBackBuffer();
	mRenderQueue->drawLights();

	mDx11Renderer->setRenderTargets(nullptr, 1);

	shaderResourceViews[3] = mLightRenderTarget->getShaderResourceView();

	mDx11Renderer->getDeviceContext()->PSSetShaderResources(0, 4, shaderResourceViews);

	//////////////////////////////////////////////////////////////////////////

	mShaderManager->setPixelShader(mMergerPixelShader);
	mShaderManager->setVertexShader(mMergerVertexShader);

	mDx11Renderer->setBackBufferAsRenderTarget();

	//Draw a fullscreen quad with the merger shader to produce final output.
	mFullScreenQuad->draw(mDx11Renderer->getDeviceContext());

	//Unbind shader resource views.
	memset(shaderResourceViews, 0, sizeof(ID3D11ShaderResourceView*)
		* ARRAYSIZE(shaderResourceViews));
	mDx11Renderer->getDeviceContext()->PSSetShaderResources(0, 4, shaderResourceViews);

	//Call the callbacks
	for (unsigned int i = 0, count = mEndOfRenderCallbacks.size(); i < count; ++i)
	{
		mEndOfRenderCallbacks[i]();
	}

	mDx11Renderer->present();

	mDx11Renderer->clearBackBuffer();
	mDx11Renderer->clearRenderTargets(&mGBuffer.position, 3);
	mDx11Renderer->clearRenderTargets(&mLightRenderTarget, 1);
}
