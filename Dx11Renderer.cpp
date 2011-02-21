#include "Dx11Renderer.h"

#include <sstream>


Dx11Renderer::~Dx11Renderer()
{
	Log::logMessage("Uninitializing DirectX", pantheios::SEV_NOTICE);

	if (mDepthStencil)		mDepthStencil->Release();
	if (mDepthStencilView)	mDepthStencilView->Release();

	if (mRenderTargetView)	mRenderTargetView->Release();

	if (mSwapChain)			mSwapChain->Release();

	if (mDeviceContext)
	{
		mDeviceContext->ClearState();
		mDeviceContext->Flush();
		mDeviceContext->Release();
	}
	if (mDevice)		mDevice->Release();


	Log::logMessage("DirectX successfully uninitialized");
}

void Dx11Renderer::init(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
{
	Log::logMessage("Starting initialization of Direct3D 11");

	try
	{
		//Swap chain & device
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = renderWindowWidth;
		swapChainDesc.BufferDesc.Height = renderWindowHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = true;

		unsigned int deviceFlags(0);
#ifdef _DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] = 
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
		
		Log::logMessage("Creating D3D11 device and swap chain");
		if (!SUCCEEDED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
			featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain,
			&mDevice, &fl, &mDeviceContext)))
				throw std::exception("Failed to create D3D11 device");
		
		//Back buffer
		ID3D11Texture2D *backBuffer = nullptr;
		Log::logMessage("Creating back buffer");
		if (!SUCCEEDED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
			throw std::exception("Failed to create back buffer");

		//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		Log::logMessage("Creating render target view");
		if (!SUCCEEDED(mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView)))
			throw std::exception("Failed to create render target view");

		backBuffer->Release();


		//Depth stencil
		D3D11_TEXTURE2D_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(depthDesc));

		depthDesc.Width = renderWindowWidth;
		depthDesc.Height = renderWindowHeight;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;

		Log::logMessage("Creating depth stencil texture");
		if (!SUCCEEDED(mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencil)))
			throw std::exception("Failed to create depth stencil texture");


		//Depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
		ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));

		depthViewDesc.Format = depthDesc.Format;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0;

		Log::logMessage("Creating depth stencil view");
		if (!SUCCEEDED(mDevice->CreateDepthStencilView(mDepthStencil, &depthViewDesc,
			&mDepthStencilView)))
		{
			throw std::exception("Failed to create depth stencil view");
		}


		mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


		//Viewport
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)renderWindowWidth;
		viewport.Height = (float)renderWindowHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		mDeviceContext->RSSetViewports(1, &viewport);

#ifdef _DEBUG
		//Set debug info in D3D objects to help debugging their lifetimes if necessary.

		const char* swapChainName = "IDXGISwapChain";
		mSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(swapChainName) - 1,
			swapChainName);

		const char* deviceName = "ID3D11Device";
		mDevice->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(deviceName) - 1,
			deviceName);

		const char* deviceContextName = "ID3D11DeviceContext";
		mDeviceContext->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof(deviceContextName) - 1, deviceContextName);

		const char* renderTargetViewName = "ID3D11RenderTargetView";
		mRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof(renderTargetViewName) - 1, renderTargetViewName);

		const char* depthStencilName = "ID3D11Texture2D_DepthStencil";
		mDepthStencil->SetPrivateData(WKPDID_D3DDebugObjectName, 
			sizeof(depthStencilName) - 1, depthStencilName);

		const char* depthStencilViewName = "ID3D11DepthStencilView";
		mDepthStencilView->SetPrivateData(WKPDID_D3DDebugObjectName, 
			sizeof(depthStencilViewName) - 1, depthStencilViewName);
#endif

		initDXUT(hWnd);
	}

	catch (std::exception &e)
	{
		Log::logMessage(std::string(e.what() + std::string("\nExiting")).c_str(),
			pantheios::SEV_EMERGENCY);
		MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		exit(EXIT_FAILURE);
	}	

	Log::logMessage("DirectX initialization completed successfully");
}

void Dx11Renderer::initDXUT(HWND hWnd)
{
	Log::logMessage("Initializing DXUT");

	if (!SUCCEEDED(DXUTInit()))
		throw std::exception("Failed to initialize DXUT");

	DXUTSetHotkeyHandling(false, false, false);

	if (!SUCCEEDED(DXUTSetWindow(hWnd, hWnd, hWnd, false)))
		throw std::exception("Failed to set DXUT window");

	
//	if (!SUCCEEDED(DXUTSetD3D10Device(mDevice, mSwapChain)))
//		throw std::exception("Failed to set D3D device for DXUT");

	Log::logMessage("DXUT successfully initialized");
}

void Dx11Renderer::preRender()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	
	//Clear depth buffer to max (1.0)
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Dx11Renderer::render()
{
	
}

void Dx11Renderer::present()
{
	if (!SUCCEEDED(mSwapChain->Present(0, 0)))
		Log::logMessage("IDXGISwapChain::Present failed", pantheios::SEV_ERROR);
}

HRESULT Dx11Renderer::compileShader(const char* fileName, const char* entryPoint, const char* shaderModel, ID3DBlob** blobOut)
{
	HRESULT hResult = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *errorBlob;
	hResult = D3DX11CompileFromFileA(fileName, nullptr, nullptr, entryPoint, shaderModel, shaderFlags, 0, 
		nullptr, blobOut, &errorBlob, nullptr);

	if (FAILED(hResult))
	{
		if (errorBlob != nullptr)
		{
			std::stringstream message;
			message << "Failed to compile shader " << fileName << ". Error message: " << (char*)errorBlob->GetBufferPointer();
			Log::logMessage(message.str().c_str(), pantheios::SEV_ERROR);
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		if (errorBlob)
			errorBlob->Release();

		return hResult;
	}

	if (errorBlob)
		errorBlob->Release();

	return S_OK;
}
