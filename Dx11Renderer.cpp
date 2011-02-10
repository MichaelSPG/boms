#include "Dx11Renderer.h"


Dx11Renderer::~Dx11Renderer()
{
	Log::log("Uninitializing DirectX", LOG_SEV_NOTICE);

	if (mDeviceContext)	mDeviceContext->ClearState();

	if (mSamplerLinear)	mSamplerLinear->Release();

	if (mCBChangesNever)		mCBChangesNever->Release();
	if (mCBChangesOnResize)		mCBChangesOnResize->Release();

	if (mDepthStencil)		mDepthStencil->Release();
	if (mDepthStencilView)	mDepthStencilView->Release();

	if (mRenderTargetView)	mRenderTargetView->Release();

	if (mSwapChain)			mSwapChain->Release();

	if (mDeviceContext)	mDeviceContext->Release();
	if (mDevice)		mDevice->Release();

	Log::log("DirectX successfully uninitialized");
}

void Dx11Renderer::init(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
{
	Log::log("Starting initialization of Direct3D 11");

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

		Log::log("Creating D3D11 device and swap chain");
		if (!SUCCEEDED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
			featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain,
			&mDevice, &fl, &mDeviceContext)))
				throw std::exception("Failed to create D3D11 device");

		
		//Back buffer
		ID3D11Texture2D *backBuffer = nullptr;
		Log::log("Creating back buffer");
		if (!SUCCEEDED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
			throw std::exception("Failed to create back buffer");

		//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		Log::log("Creating render target view");
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

		Log::log("Creating depth stencil texture");
		if (!SUCCEEDED(mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencil)))
			throw std::exception("Failed to create depth stencil texture");


		//Depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
		ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));

		depthViewDesc.Format = depthDesc.Format;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0;

		Log::log("Creating depth stencil view");
		if (!SUCCEEDED(mDevice->CreateDepthStencilView(mDepthStencil, &depthViewDesc, &mDepthStencilView)))
			throw std::exception("Failed to create depth stencil view");


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


		D3D11_BUFFER_DESC bufferDescription;
		ZeroMemory(&bufferDescription, sizeof(bufferDescription));

		//Constant buffers
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(CBChangesNever);
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = 0;

		if (!SUCCEEDED(mDevice->CreateBuffer(&bufferDescription, nullptr, &mCBChangesNever)))
			throw std::exception("Failed to create constant buffer");

		bufferDescription.ByteWidth = sizeof(CBChangesOnResize);
		if (!SUCCEEDED(mDevice->CreateBuffer(&bufferDescription, nullptr, &mCBChangesOnResize)))
			throw std::exception("Failed to create constant buffer");


		bufferDescription.ByteWidth = sizeof(CBChangesEveryFrame);
		if (!SUCCEEDED(mDevice->CreateBuffer(&bufferDescription, nullptr, &mCBChangesEveryFrame)))
			throw std::exception("Failed to create constant buffer");


/*

		//Texture
		if (!SUCCEEDED(D3DX11CreateShaderResourceViewFromFile(mDevice, "seafloor.dds", nullptr, nullptr,
			&mTextureRV, nullptr)))
			throw std::exception("Failed to create shader resource from file");
*/

/*

		//Sample state
		D3D11_SAMPLER_DESC sampleDescription;
		ZeroMemory(&sampleDescription, sizeof(sampleDescription));

		sampleDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampleDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDescription.MinLOD = 0;
		sampleDescription.MaxLOD = D3D11_FLOAT32_MAX;

		if (!SUCCEEDED(mDevice->CreateSamplerState(&sampleDescription, &mSamplerLinear)))
			throw std::exception("Failed to create sampler state");
*/

		//View matrix
		//mWorld = XMMatrixIdentity();
		XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
		/*
		auto translation = XMMatrixTranslation(10.0f, 20.0f, 30.0f);
		auto rotation = XMMatrixRotationX(90);

		XMFLOAT3 trans = Math::getTranslationFromMatrix(translation);
		*/

		XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
		XMVECTOR At =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR Up =  XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		//mView = XMMatrixLookAtLH(Eye, At, Up);

		XMStoreFloat4x4(&mView, XMMatrixLookAtLH(Eye, At, Up));

		CBChangesNever cbChangesNever;
		XMStoreFloat4x4(&cbChangesNever.mView, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
		//cbChangesNever.mView = XMMatrixTranspose(mView);
		mDeviceContext->UpdateSubresource(mCBChangesNever, 0, nullptr, &cbChangesNever, 0, 0);


		//Projection matrix
//		mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, renderWindowWidth / (float)renderWindowHeight,
//			0.01f, 100.0f);
		XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(XM_PIDIV4 /*XMConvertToRadians(90.0f)*/, renderWindowWidth /
			(float)renderWindowHeight, 0.01f, 100.0f));

		CBChangesOnResize cbChangesOnResize;
		XMStoreFloat4x4(&cbChangesOnResize.mProjection, XMMatrixTranspose(XMLoadFloat4x4(&mProjection)));
		//cbChangesOnResize.mProjection = XMMatrixTranspose(mProjection);
		mDeviceContext->UpdateSubresource(mCBChangesOnResize, 0, nullptr, &cbChangesOnResize, 0, 0);


		initDXUT(hWnd);
	}

	catch (std::exception &e)
	{
		Log::log(std::string(e.what() + std::string("\nExiting")).c_str(), LOG_SEV_EMERGENCY);
		MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		exit(EXIT_FAILURE);
	}	

	Log::log("DirectX initialization completed successfully");
}

void Dx11Renderer::initDXUT(HWND hWnd)
{
	Log::log("Initializing DXUT");

	if (!SUCCEEDED(DXUTInit()))
		throw std::exception("Failed to initialize DXUT");

	DXUTSetHotkeyHandling(false, false, false);

	if (!SUCCEEDED(DXUTSetWindow(hWnd, hWnd, hWnd, false)))
		throw std::exception("Failed to set DXUT window");

	
//	if (!SUCCEEDED(DXUTSetD3D10Device(mDevice, mSwapChain)))
//		throw std::exception("Failed to set D3D device for DXUT");

	Log::log("DXUT successfully initialized");
}

void Dx11Renderer::preRender()
{
	//Clear back buffer.
	/*float random[4] =
	{
		cosf(t),
		sinf(t),
		-cosf(t),
		sinf(t)
	};
	*/

	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	//mDeviceContext->ClearRenderTargetView(mRenderTargetView, random);
	
	//Clear depth buffer to max (1.0)
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	static float t(0.0f);
	{
		static DWORD timeStart(0);
		DWORD timeCur = GetTickCount();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}


	/*
	
	XMMATRIX rotX = XMMatrixRotationX(t * 0.01f);
	XMMATRIX rotY = XMMatrixRotationY(t * 0.01f);
	XMMATRIX rotZ = XMMatrixRotationZ(sinf(t));
	XMMATRIX rot = XMMatrixMultiply(XMMatrixMultiply(rotX, rotY), rotZ);
	
	XMMATRIX trix = XMLoadFloat4x4(&mView);
	trix = XMMatrixMultiply(trix, rotY);

	//XMStoreFloat4x4(&mView, rot);
	XMMATRIX trans = XMMatrixTranslation(0.0f, 0.0f, -25.0f);
	trix = XMMatrixMultiply(trans, trix);

	XMMATRIX pos = XMMatrixMultiply(XMLoadFloat4x4(&mView), XMMatrixTranslation(0.0f, 0.0f, -10.0f));
	XMStoreFloat4x4(&mView, pos);

	//XMStoreFloat4x4(&mView, trix);

	//XMStoreFloat4x4(&mView, XMMatrixTranslation(0.0f, 0.0f, -0.5f));

	CBChangesEveryFrame view;
	XMStoreFloat4x4(&view.mWorld, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
	
	//constBuffer.mWorld = XMMatrixTranspose(mWorld);
	//view.vMeshColor = mMeshColor;
	//mDeviceContext->UpdateSubresource(mCBChangesNever, 0, nullptr, &view, 0, 0);
	
	XMStoreFloat4x4(&mWorld, XMMatrixTranslation(0.0f, 0.0f, 0.0f));//rot);
	/*
	mMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
	mMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
	mMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;
	*/

	/*
	Math::setTranslationInFloat4x4(mView, XMFLOAT3(0.0f, 0.0f, t));

	CBChangesNever cbNever;
	XMStoreFloat4x4(&cbNever.mView, XMMatrixTranspose(XMLoadFloat4x4(&mView)));
	mDeviceContext->UpdateSubresource(mCBChangesNever, 0, nullptr, &cbNever, 0, 0);
	*/
	
	CBChangesEveryFrame constBuffer;
	XMStoreFloat4x4(&constBuffer.mWorld, XMMatrixTranspose(XMLoadFloat4x4(&mWorld)));
	//constBuffer.mWorld = XMMatrixTranspose(mWorld);
	//constBuffer.vMeshColor = mMeshColor;
	mDeviceContext->UpdateSubresource(mCBChangesEveryFrame, 0, nullptr, &constBuffer, 0, 0);
	
	static bool once = false;
	if (!once)
	{
	//	mDeviceContext->VSSetConstantBuffers(0, 1, &mCBChangesNever);
		mDeviceContext->VSSetConstantBuffers(1, 1, &mCBChangesOnResize);
		//mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		mDeviceContext->UpdateSubresource(mCBChangesEveryFrame, 0, nullptr, &constBuffer, 0, 0);
		once = true;
	}


	//mDeviceContext->PSSetConstantBuffers(2, 1, &mCBChangesEveryFrame);
	//mDeviceContext->PSSetShaderResources(0, 1, &mTextureRV);
	//mDeviceContext->PSSetSamplers(0, 1, &mSamplerLinear);
	//mDeviceContext->PSSetSamplers(0, 0, nullptr);
}

void Dx11Renderer::render()
{
	
}

void Dx11Renderer::present()
{
	if (!SUCCEEDED(mSwapChain->Present(0, 0)))
		Log::log("IDXGISwapChain::Present failed", LOG_SEV_ERROR);
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
			Log::log(message.str().c_str(), LOG_SEV_ERROR);
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
