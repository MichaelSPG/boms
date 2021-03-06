#include "StdAfx.h"

#include "bsDeferredRenderer.h"

#include <vector>

#include <d3d11.h>

#include "bsDx11Renderer.h"
#include "bsRenderQueue.h"
#include "bsRenderTarget.h"
#include "bsWindow.h"
#include "bsFullScreenQuad.h"
#include "bsShaderManager.h"
#include "bsAssert.h"
#include "bsTimer.h"
#include "bsFrameStatistics.h"
#include "bsFixedSizeString.h"


bsDeferredRenderer::bsDeferredRenderer(bsDx11Renderer* dx11Renderer,
	bsShaderManager* shaderManager, bsWindow* window, bsRenderQueue* renderQueue)
	: mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
	, mFxaaPass(shaderManager, dx11Renderer)
	, mGeometryRasterizerState(nullptr)
	, mRenderQueue(renderQueue)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(shaderManager);

	ID3D11Device& device = *mDx11Renderer->getDevice();
	ID3D11DeviceContext& deviceContext = *mDx11Renderer->getDeviceContext();

	const int windowWidth = window->getWindowWidth(), windowHeight = window->getWindowHeight();
	
	mGBuffer.position	= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.normal		= new bsRenderTarget(windowWidth, windowHeight, device);
	mGBuffer.diffuse	= new bsRenderTarget(windowWidth, windowHeight, device);
	mLightRenderTarget	= new bsRenderTarget(windowWidth, windowHeight, device);
	mFinalRenderTarget	= new bsRenderTarget(windowWidth, windowHeight, device);

	mDx11Renderer->registerRenderTarget(*mGBuffer.position);
	mDx11Renderer->registerRenderTarget(*mGBuffer.normal);
	mDx11Renderer->registerRenderTarget(*mGBuffer.diffuse);
	mDx11Renderer->registerRenderTarget(*mLightRenderTarget);
	mDx11Renderer->registerRenderTarget(*mFinalRenderTarget);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	memset(&rasterizerDesc, 0, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthClipEnable = true;

	device.CreateRasterizerState(&rasterizerDesc, &mGeometryRasterizerState);

	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	device.CreateRasterizerState(&rasterizerDesc, &mCullBackFacingNoDepthClip);

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	device.CreateRasterizerState(&rasterizerDesc, &mCullFrontFacingNoDepthClip);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	device.CreateRasterizerState(&rasterizerDesc, &mCullNoneNoDepthClip);


	deviceContext.RSSetState(mGeometryRasterizerState);

	mFullScreenQuad = new bsFullScreenQuad(&device);

	createShaders();

	//Blending states
	D3D11_BLEND_DESC blendDesc;
	memset(&blendDesc, 0, sizeof(blendDesc));
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hresult = device.CreateBlendState(&blendDesc, &mGeometryBlendState);
	BS_ASSERT2(SUCCEEDED(hresult), "Failed to create blend state");

	/*
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;//D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	*/
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	hresult = device.CreateBlendState(&blendDesc, &mLightBlendState);
	BS_ASSERT2(SUCCEEDED(hresult), "Failed to create blend state");

	deviceContext.OMSetBlendState(mGeometryBlendState, nullptr, 0xFFFFFFFF);


	//Depth stencils
	D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
	memset(&depthStencilDescription, 0, sizeof(depthStencilDescription));
	depthStencilDescription.DepthEnable = true;
	depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDescription.StencilEnable = false;
	depthStencilDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDescription.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	depthStencilDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	depthStencilDescription.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
	depthStencilDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;
	
	device.CreateDepthStencilState(&depthStencilDescription, &mDepthEnabledStencilState);

	depthStencilDescription.DepthEnable = false;
	depthStencilDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;
	device.CreateDepthStencilState(&depthStencilDescription, &mDepthDisabledStencilState);

	deviceContext.OMSetDepthStencilState(mDepthEnabledStencilState, 0);

	


#ifdef BS_DEBUG
	//Set names for debugging purposes
	//Position
	bsString128 debugData = "SRV GBuffer Position";
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

	//Final target
	debugData = "SRV Final";
	mFinalRenderTarget->getShaderResourceView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTV Final";
	mFinalRenderTarget->getRenderTargetView()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	debugData = "RTT Final";
	mFinalRenderTarget->getRenderTargetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName,
		debugData.size(), debugData.c_str());

	//Rasterizer states
	debugData = "RasterizerState Geometry";
	mGeometryRasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	debugData = "RasterizerState Cull backfacing";
	mCullBackFacingNoDepthClip->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	debugData = "RasterizerState Cull frontfacing";
	mCullFrontFacingNoDepthClip->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	debugData = "RasterizerState Cull none no depth clip";
	mCullNoneNoDepthClip->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	//Blend states
	debugData = "BlendState Geometry";
	mGeometryBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	debugData = "BlendState Light";
	mLightBlendState->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	//Depth stencil states
	debugData = "DepthStencil Depth enabled";
	mDepthEnabledStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());

	debugData = "DepthStencil Depth disabled";
	mDepthDisabledStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, debugData.size(),
		debugData.c_str());
#endif
}

bsDeferredRenderer::~bsDeferredRenderer()
{
	mGeometryRasterizerState->Release();
	mCullBackFacingNoDepthClip->Release();
	mCullFrontFacingNoDepthClip->Release();
	mCullNoneNoDepthClip->Release();

	mGeometryBlendState->Release();
	mLightBlendState->Release();

	mDepthEnabledStencilState->Release();
	mDepthDisabledStencilState->Release();

	delete mFullScreenQuad;

	delete mFinalRenderTarget;
	delete mLightRenderTarget;
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

	mMergerVertexShader = mShaderManager->getVertexShader("Merger.fx", inputLayout.data(), inputLayout.size());
	mMergerPixelShader = mShaderManager->getPixelShader("Merger.fx");
}

void bsDeferredRenderer::renderOneFrame(bsFrameStatistics& frameStatistics)
{
	bsTimer timer;

	float preCull, cullDuration,
		preOpaque, opaqueDuration,
		preTransparent, transparentDuration,//Normal transparent geometry + 3D texts
		preLines, linesDuration,
		preLight, lightDuration,
		preAccumulate, accumulateDuration,
		preFxaa, fxaaDuration,
		prePresent, presentDuration,
		preStateChange, stateChangeDuration;

	ID3D11DeviceContext* deviceContext = mDx11Renderer->getDeviceContext();

	mRenderQueue->reset();

	preCull = timer.getTimeMilliSeconds();
	{
		mRenderQueue->startFrame();
	}
	cullDuration = timer.getTimeMilliSeconds() - preCull;

	//Set and clear G buffer
	preStateChange = timer.getTimeMilliSeconds();
	{
		mDx11Renderer->setRenderTargets(&mGBuffer.position, 3);

		//Enable depth testing.
		deviceContext->OMSetDepthStencilState(mDepthEnabledStencilState, 0);
	}
	stateChangeDuration = timer.getTimeMilliSeconds() - preStateChange;

	preOpaque = timer.getTimeMilliSeconds();
	{
		//Draw the geometry into the G buffers.
		mRenderQueue->drawGeometry();
	}
	opaqueDuration = timer.getTimeMilliSeconds() - preOpaque;

	preStateChange = timer.getTimeMilliSeconds();
	{
		//Unbind G buffer render targets.
		mDx11Renderer->setRenderTargets(nullptr, 3);

		//Set the GBuffer as shader resources, allowing them to be used as input by shaders
	}
		ID3D11ShaderResourceView* shaderResourceViews[4];
	{
		shaderResourceViews[0] = mGBuffer.position->getShaderResourceView();
		shaderResourceViews[1] = mGBuffer.normal->getShaderResourceView();
		shaderResourceViews[2] = mGBuffer.diffuse->getShaderResourceView();
		deviceContext->PSSetShaderResources(0, 3, shaderResourceViews);

		//////////////////////////////////////////////////////////////////////////
		//Render lights

		//Set the light render target.
		mDx11Renderer->setRenderTargets(&mLightRenderTarget, 1);

		//Enable light rasterization, ie no cull backfacing
		deviceContext->RSSetState(mCullFrontFacingNoDepthClip);
		//Enable blending
		deviceContext->OMSetBlendState(mLightBlendState, nullptr, 0xFFFFFFFF);
		//Disable depth
		deviceContext->OMSetDepthStencilState(mDepthDisabledStencilState, 0);
	}
	stateChangeDuration += timer.getTimeMilliSeconds() - preStateChange;

	
	preLight = timer.getTimeMilliSeconds();
	{
		mRenderQueue->drawLights();
	}
	lightDuration = timer.getTimeMilliSeconds() - preLight;

	deviceContext->OMSetDepthStencilState(mDepthEnabledStencilState, 0);

	//Draw lines here since we don't want them to be affected by lights.
	preLines = timer.getTimeMilliSeconds();
	{
		deviceContext->OMSetBlendState(mGeometryBlendState, nullptr, 0xFFFFFFFF);
		mRenderQueue->drawLines();
		deviceContext->OMSetBlendState(mLightBlendState, nullptr, 0xFFFFFFFF);
	}
	linesDuration = timer.getTimeMilliSeconds() - preLines;

	preTransparent = timer.getTimeMilliSeconds();
	{
		mRenderQueue->drawTexts();
	}
	transparentDuration = timer.getTimeMilliSeconds() - preTransparent;

	deviceContext->OMSetDepthStencilState(mDepthDisabledStencilState, 0);

	//////////////////////////////////////////////////////////////////////////
	//
	preStateChange = timer.getTimeMilliSeconds();
	{
		mDx11Renderer->setRenderTargets(&mFinalRenderTarget, 1);

		shaderResourceViews[3] = mLightRenderTarget->getShaderResourceView();

		deviceContext->PSSetShaderResources(0, 4, shaderResourceViews);

		//////////////////////////////////////////////////////////////////////////

		mShaderManager->setPixelShader(mMergerPixelShader);
		mShaderManager->setVertexShader(mMergerVertexShader);

		//Enable geometry rasterization for fullscreen quad and next geometry pass
		deviceContext->RSSetState(mGeometryRasterizerState);
		deviceContext->OMSetBlendState(mGeometryBlendState, nullptr, 0xFFFFFFFF);
	}
	stateChangeDuration += timer.getTimeMilliSeconds() - preStateChange;

	//mDx11Renderer->setBackBufferAsRenderTarget();
	//Draw a fullscreen quad with the merger shader to produce final output.
	preAccumulate = timer.getTimeMilliSeconds();
	{
		mFullScreenQuad->draw(mDx11Renderer->getDeviceContext());
	}
	accumulateDuration = timer.getTimeMilliSeconds() - preAccumulate;


	//////////////////////////////////////////////////////////////////////////
	//FXAA

	preStateChange = timer.getTimeMilliSeconds();
	//Unbind previous render target.
	{
		mDx11Renderer->setRenderTargets(nullptr, 1);

		mDx11Renderer->setBackBufferAsRenderTarget();

		shaderResourceViews[0] = mFinalRenderTarget->getShaderResourceView();
		deviceContext->PSSetShaderResources(0, 1, shaderResourceViews);
	}
	stateChangeDuration += timer.getTimeMilliSeconds() - preAccumulate;

	preFxaa = timer.getTimeMilliSeconds();
	{
		mFxaaPass.draw();
	}
	fxaaDuration = timer.getTimeMilliSeconds() - preFxaa;


	//deviceContext->OMSetDepthStencilState(mDepthEnabledStencilState, 0);
	//deviceContext->RSSetState(mGeometryRasterizerState);
	//mRenderQueue->drawLines();

	preAccumulate = timer.getTimeMilliSeconds();
	//Unbind shader resource views.
	{
		memset(shaderResourceViews, 0, sizeof(ID3D11ShaderResourceView*)
			* ARRAYSIZE(shaderResourceViews));
		deviceContext->PSSetShaderResources(0, 4, shaderResourceViews);
	}
	accumulateDuration += timer.getTimeMilliSeconds() - preAccumulate;

	//Call the callbacks
	for (unsigned int i = 0, count = mEndOfRenderCallbacks.size(); i < count; ++i)
	{
		mEndOfRenderCallbacks[i](frameStatistics);
	}

	prePresent = timer.getTimeMilliSeconds();
	{
		mDx11Renderer->present();
	}
	presentDuration = timer.getTimeMilliSeconds() - prePresent;

	preAccumulate = timer.getTimeMilliSeconds();
	{
		mDx11Renderer->clearBackBuffer();
		mDx11Renderer->clearRenderTargets(&mGBuffer.position, 3);
		mDx11Renderer->clearRenderTargets(&mLightRenderTarget, 1);
		mDx11Renderer->clearRenderTargets(&mFinalRenderTarget, 1);
	}
	accumulateDuration += timer.getTimeMilliSeconds() - preAccumulate;


	//Write frame timings.

	frameStatistics.renderingInfo.cullDuration = cullDuration;
	frameStatistics.renderingInfo.opaqueDuration = opaqueDuration;
	frameStatistics.renderingInfo.transparentDuration = transparentDuration;
	frameStatistics.renderingInfo.lightDuration = lightDuration;
	frameStatistics.renderingInfo.lineDuration = linesDuration;
	frameStatistics.renderingInfo.accumulateDuration = accumulateDuration;
	frameStatistics.renderingInfo.fxaaDuration = fxaaDuration;

	frameStatistics.renderingInfo.presentDuration = presentDuration;
	frameStatistics.renderingInfo.additionalStateChangeOverheadDuration = stateChangeDuration;
}
