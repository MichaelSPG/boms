#include "StdAfx.h"

#include "bsRenderQueue.h"

#include <algorithm>

#include "bsCamera.h"
#include "bsEntity.h"
#include "bsMesh.h"
#include "bsMeshRenderer.h"
#include "bsLineRenderer.h"
#include "bsLight.h"

#include "bsDx11Renderer.h"
#include "bsShaderManager.h"
#include "bsVertexShader.h"
#include "bsPixelShader.h"
#include "bsLog.h"
#include "bsTimer.h"
#include "bsAssert.h"
#include "bsText3D.h"

#include "bsConstantBuffers.h"
#include "bsFrustum.h"

#include "bsAlignedAllocator.h"
#include "bsFixedSizeString.h"


bsRenderQueue::bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
	: mCamera(nullptr)
	, mScene(nullptr)
	, mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(shaderManager);


	ID3D11Device& device = *mDx11Renderer->getDevice();

	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(XMMATRIX);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;
	bufferDescription.MiscFlags = 0;

	HRESULT hres = device.CreateBuffer(&bufferDescription, nullptr, &mWorldBuffer);

	BS_ASSERT2(SUCCEEDED(hres), "bsRenderQueue::bsRenderQueue failed to create world buffer");

	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	hres = device.CreateBuffer(&bufferDescription, nullptr, &mWireframeWorldBuffer);
	BS_ASSERT(SUCCEEDED(hres));

	bufferDescription.ByteWidth = sizeof(CBLight);
	hres = device.CreateBuffer(&bufferDescription, nullptr, &mLightBuffer);
	BS_ASSERT(SUCCEEDED(hres));

	//Shaders
	//Wireframe
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC inputElementDesc;
	ZeroMemory(&inputElementDesc, sizeof(inputElementDesc));
	inputElementDesc.SemanticName = "POSITION";
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc.InputSlot = 0;
	inputElementDesc.AlignedByteOffset = 0;
	inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc.InstanceDataStepRate = 0;
	inputLayout.push_back(inputElementDesc);

	mWireframeVertexShader = mShaderManager->getVertexShader("Wireframe.fx",
		inputLayout.data(), inputLayout.size());
	mWireframePixelShader = mShaderManager->getPixelShader("Wireframe.fx");


	D3D11_INPUT_ELEMENT_DESC layout[8] =
	{
		//Vertex data.
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		//Instance data (4x4 matrix).
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	mMeshInstancedVertexShader = mShaderManager->getVertexShader("MeshInstanced.fx", layout, 8);
	mInstancedTexturedMeshPixelShader = mShaderManager->getPixelShader("MeshInstancedTextured.fx");
	mInstancedTexturedMeshNormalPixelShader = mShaderManager->getPixelShader("MeshInstancedTexturedNormal.fx");

	//Light
	inputLayout.clear();
	inputElementDesc.SemanticName = "POSITION";
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc.AlignedByteOffset = 0;
	inputLayout.push_back(inputElementDesc);

	mLightVertexShader = mShaderManager->getVertexShader("Light.fx", inputLayout.data(),
		inputLayout.size());
	mLightPixelShader = mShaderManager->getPixelShader("Light.fx");

	D3D11_INPUT_ELEMENT_DESC lightInstanced[8] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		//Instance data (4x4 matrix + pos/radius/color/intensity).
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		//Position/radius
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		//Color/intensity
		{ "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		//Direction/spot cone
		{ "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	mLightInstancedVertexShader = mShaderManager->getVertexShader("LightInstanced.fx",
		lightInstanced, ARRAYSIZE(lightInstanced));
	mPointLightInstancedPixelShader = mShaderManager->getPixelShader("LightInstanced.fx");
	mSpotLightInstancedPixelShader = mShaderManager->getPixelShader("SpotLightInstanced.fx");

	//Create material buffer.
	bufferDescription.ByteWidth = sizeof(XMFLOAT4) * 2;
	hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mMaterialBuffer);
	BS_ASSERT2(SUCCEEDED(hres), "Failed to create material buffer");


	D3D11_SAMPLER_DESC lightSamplerDesc;
	lightSamplerDesc.AddressU = lightSamplerDesc.AddressV = lightSamplerDesc.AddressW =
		D3D11_TEXTURE_ADDRESS_CLAMP;
	lightSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	lightSamplerDesc.MinLOD = 0.0f;
	lightSamplerDesc.MaxLOD = 0.0f;
	lightSamplerDesc.MipLODBias = 0.0f;
	lightSamplerDesc.MaxAnisotropy = 16;
	lightSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	lightSamplerDesc.BorderColor[0] = lightSamplerDesc.BorderColor[1] =
		lightSamplerDesc.BorderColor[2] = lightSamplerDesc.BorderColor[3] = 0.0f;

	hres = device.CreateSamplerState(&lightSamplerDesc, &mLightSamplerState);
	BS_ASSERT2(SUCCEEDED(hres), "Failed to create light sampler state");


#ifdef BS_DEBUG
	bsString128 debugName("bsRenderQueue world buffer");
	mWorldBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(),
		debugName.c_str());

	debugName = "bsRenderQueue wireframe world buffer";
	mWireframeWorldBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(),
		debugName.c_str());

	debugName = "bsRenderQueue light buffer buffer";
	mLightBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(),
		debugName.c_str());

	debugName = "bsRenderQueue material buffer";
	mMaterialBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(),
		debugName.c_str());
#endif
}

bsRenderQueue::~bsRenderQueue()
{
	mWorldBuffer->Release();
	mWireframeWorldBuffer->Release();
	mLightBuffer->Release();
}

void bsRenderQueue::reset()
{
	mFrameStats.reset();

	mMeshesToDraw.clear();
	mLinesToDraw.clear();
	
	mPointLightPositionPairs.clear();
	mSpotLightPositionPairs.clear();
	mDirectionalLightPositionPairs.clear();

	mText3dToDraw.clear();
}

void bsRenderQueue::startFrame()
{
	BS_ASSERT2(mScene != nullptr, "startFrame called, but no scene has been registered");

	const std::vector<bsEntity*>& entities = mScene->getEntities();
	std::vector<const bsEntity*> entities2(entities.begin(), entities.end());

	addAndCullObjects(entities2.data(), entities2.size(), mScene->getCamera()->getTransformedFrustum());
}

void bsRenderQueue::addAndCullObjects(const bsEntity** entities, unsigned int entityCount,
	const bsFrustum& frustum)
{
	std::vector<const bsEntity*> visibleEntities;
	visibleEntities.reserve(entityCount / 4);

	//Add entities that are inside/intersecting the frustum to the list of visible entities.
	for (unsigned int i = 0; i < entityCount; ++i)
	{
		const bsEntity& entity = *entities[i];
		if (bsCollision::intersectSphereFrustum(entity.getBoundingSphere(),
			entity.getTransform().getPosition(), frustum)
			!= bsCollision::OUTSIDE)
		{
			visibleEntities.push_back(&entity);
		}
	}

	sortRenderables(visibleEntities.data(), visibleEntities.size());
}

void bsRenderQueue::drawGeometry()
{
	unbindGeometryShader();

	//sortLights();

	//Render all the geometric renderable types
	drawMeshesInstanced();
}

void bsRenderQueue::setWorldConstantBuffer(const XMMATRIX& world)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(1, 1, &mWorldBuffer);
	context->PSSetConstantBuffers(1, 1, &mWorldBuffer);

	context->UpdateSubresource(mWorldBuffer, 0, nullptr, &world, 0, 0);
}

void bsRenderQueue::setWireframeConstantBuffer(const XMMATRIX& world, const XMFLOAT4& color)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(1, 1, &mWireframeWorldBuffer);
	context->PSSetConstantBuffers(1, 1, &mWireframeWorldBuffer);

	CBWireFrame cbWireFrame;
	//XMStoreFloat4x4(&cbWireFrame.world, world);
	cbWireFrame.world = world;
	XMFLOAT4A colorAligned;
	memcpy(&colorAligned, &color, sizeof(XMFLOAT4));
	cbWireFrame.color = colorAligned;

	context->UpdateSubresource(mWireframeWorldBuffer, 0, nullptr, &cbWireFrame, 0, 0);
}

void bsRenderQueue::setLightConstantBuffer(const CBLight& cbLight)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(3, 1, &mLightBuffer);
	context->PSSetConstantBuffers(3, 1, &mLightBuffer);

	context->UpdateSubresource(mLightBuffer, 0, nullptr, &cbLight, 0, 0);
}

void bsRenderQueue::sortRenderables(const bsEntity** entities, unsigned int entityCount)
{
	//BS_ASSERT2(mCamera, "Camera must be set before attempting to render a frame");

	//std::vector<bsEntity*> entities = mCamera->getVisibleEntities();

	//mFrameStats.visibleEntityCount = entities.size();
	mFrameStats.visibleEntityCount += entityCount;
	

	//Gather all the renderables from the visible meshEntities.
	for (unsigned int i = 0, count = entityCount; i < count; ++i)
	{
		const bsEntity& entity = *entities[i];

		const bsMeshRenderer* meshRenderer = entity.getMeshRenderer();

		if (meshRenderer)
		{
			auto finder = mMeshesToDraw.find(meshRenderer);

			for (auto itr = mMeshesToDraw.begin(), end = mMeshesToDraw.end(); itr != end; ++itr)
			{
				if (itr->first->getUniqueID() == meshRenderer->getUniqueID())
				{
					finder = itr;
					break;
				}
			}


			if (finder == mMeshesToDraw.end())
			{
				//Not found, create it
				std::vector<const bsEntity*> meshEntities(1);
				meshEntities[0] = &entity;
				mMeshesToDraw.insert(std::make_pair(meshRenderer, meshEntities));
			}
			else
			{
				//Found, add the entity to the mesh' list of entities
				finder->second.push_back(&entity);
			}
		}

		const bsLineRenderer* line = entity.getLineRenderer();
		if (line != nullptr)
		{
			auto finder = mLinesToDraw.find(line);

			if (finder == mLinesToDraw.end())
			{
				//Not found, create it
				std::vector<const bsEntity*> lineEntities(1);
				lineEntities[0] = &entity;
				mLinesToDraw.insert(std::make_pair(line, lineEntities));
			}
			else
			{
				//Found
				finder->second.push_back(&entity);
			}
		}

		const bsLight* light = entity.getLight();
		if (light != nullptr)
		{
			XMFLOAT3 position;
			XMStoreFloat3(&position, entity.getTransform().getPosition());
			switch (light->getLightType())
			{
			case bsLight::LT_POINT:
				mPointLightPositionPairs.push_back(std::make_pair(light, position));
				break;

			case bsLight::LT_SPOT:
				mSpotLightPositionPairs.push_back(std::make_pair(light, &entity));
				break;

			case bsLight::LT_DIRECTIONAL:
				mDirectionalLightPositionPairs.push_back(std::make_pair(light, position));
				break;
			}
		}

		const bsText3D* text = entity.getTextRenderer();
		if (text != nullptr)
		{
			mText3dToDraw.push_back(std::make_pair(&entity, text));
		}
	}

	mFrameStats.visibleLights = mPointLightPositionPairs.size();
}

void bsRenderQueue::unbindGeometryShader()
{
	mDx11Renderer->getDeviceContext()->GSSetShader(nullptr, nullptr, 0);
}

void bsRenderQueue::drawMeshesInstanced()
{
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShaderManager->setVertexShader(mMeshInstancedVertexShader);

	mFrameStats.uniqueMeshesDrawn = mMeshesToDraw.size();


	std::vector<XMMATRIX, bsAlignedAllocator<XMMATRIX>> transforms;

	for (auto itr = mMeshesToDraw.begin(), end = mMeshesToDraw.end(); itr != end; ++itr)
	{
		const bsMeshRenderer& meshRenderer = *itr->first;
		if (!meshRenderer.hasFinishedLoading())
		{
			continue;
		}

		const std::vector<const bsEntity*>& entities = itr->second;
		mFrameStats.totalMeshesDrawn += entities.size();
		mFrameStats.totalTrianglesDrawn += meshRenderer.getTriangleCount();
		mFrameStats.totalTrianglesDrawnNotInstanced += meshRenderer.getTriangleCount() * entities.size();

		transforms.clear();
		transforms.reserve(entities.size());
		for (unsigned int i = 0; i < entities.size(); ++i)
		{
			transforms.push_back(entities[i]->getTransform().getTransposedTransform());
		}

		if (meshRenderer.getMaterial()->normal)
		{
			mShaderManager->setPixelShader(mInstancedTexturedMeshNormalPixelShader);
		}
		else
		{
			mShaderManager->setPixelShader(mInstancedTexturedMeshPixelShader);
		}

		drawMeshInstanced(meshRenderer, transforms.data(), transforms.size());
	}
}

void bsRenderQueue::drawMeshInstanced(const bsMeshRenderer& meshRenderer,
	const XMMATRIX* transforms, unsigned int transformCount)
{
	//Create instance buffer. TODO: Find a less terrible way to do this.

	D3D11_BUFFER_DESC instanceBufferDesc = { 0 };
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(XMMATRIX) * transformCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA instanceData = { 0 };
	instanceData.pSysMem = transforms;

	ID3D11Buffer* instanceBuffer;
	HRESULT hr = mDx11Renderer->getDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
	BS_ASSERT(SUCCEEDED(hr));

#ifdef BS_DEBUG
	bsString32 debugName("Mesh instance buffer");
	instanceBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(), debugName.c_str());
#endif

	ID3D11DeviceContext& deviceContext = *mDx11Renderer->getDeviceContext();

	CBMaterial materialContent;
	materialContent.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	materialContent.uvTile = meshRenderer.getMaterial()->uvTile;
	deviceContext.UpdateSubresource(mMaterialBuffer, 0, nullptr, &materialContent, 0, 0);

	deviceContext.VSSetConstantBuffers(5, 1, &mMaterialBuffer);


	meshRenderer.drawInstanced(deviceContext, instanceBuffer, transformCount);


	instanceBuffer->Release();
}

void bsRenderQueue::drawLines()
{
	mShaderManager->setPixelShader(mWireframePixelShader);
	mShaderManager->setVertexShader(mWireframeVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		//D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	static float angle = 0.0f;
	angle += XMConvertToRadians(2.0f);
	if (angle > XM_PI * 2.0f)
	{
		angle = 0.0f;
	}

	for (auto itr = mLinesToDraw.begin(), end = mLinesToDraw.end(); itr != end; ++itr)
	{
		const bsLineRenderer* currentLine = itr->first;
		const std::vector<const bsEntity*>& entities = itr->second;

		mFrameStats.linesDrawn += entities.size();

		for (unsigned int i = 0, count = entities.size(); i < count; ++i)
		{
			/*
			const hkTransform& transform = entities[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 world(f4x4);
			bsMath::XMFloat4x4Transpose(world);
			*/

			const XMMATRIX position = XMMatrixTranslationFromVector(entities[i]->getTransform().getPosition());
			XMMATRIX rotation = XMMatrixRotationQuaternion(entities[i]->getTransform().getRotation());
			const XMMATRIX extraRot = XMMatrixRotationY(angle);

			//rotation = XMMatrixMultiply(rotation, extraRot);
			//rotation = extraRot;

			//rotation = XMMatrixRotationQuaternion(mCamera->getEntity()->getOwner()->getRotation());
			//XMVECTOR determinant;
			//rotation = XMMatrixInverse(&determinant, rotation);
			
			setWireframeConstantBuffer(XMMatrixMultiplyTranspose(rotation, position), currentLine->mColor);

			currentLine->draw(mDx11Renderer);
		}
	}
}

void drawInstancedLight(const bsDx11Renderer& renderer, const bsLight& light,
	const LightInstanceData* lightInstanceData, unsigned int instanceCount)
{
	D3D11_BUFFER_DESC instanceBufferDesc = { 0 };
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(LightInstanceData) * instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA instanceData = { 0 };
	instanceData.pSysMem = lightInstanceData;

	ID3D11Buffer* instanceBuffer;
	HRESULT hr = renderer.getDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
	BS_ASSERT(SUCCEEDED(hr));

#ifdef BS_DEBUG
	bsString32 debugName("Light instance buffer");
	instanceBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, debugName.size(), debugName.c_str());
#endif

	//bsLight::drawInstanced(*renderer.getDeviceContext(), instanceBuffer, instanceCount);
	light.drawInstanced(*renderer.getDeviceContext(), instanceBuffer, instanceCount);

	instanceBuffer->Release();
}

void bsRenderQueue::drawLights()
{
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mDx11Renderer->getDeviceContext()->PSSetSamplers(3, 1, &mLightSamplerState);

	mShaderManager->setVertexShader(mLightInstancedVertexShader);

	if (!mPointLightPositionPairs.empty())
	{
		mShaderManager->setPixelShader(mPointLightInstancedPixelShader);
		drawPointLights();
	}

	if (!mSpotLightPositionPairs.empty())
	{
		mShaderManager->setPixelShader(mSpotLightInstancedPixelShader);
		drawSpotLights();
	}

	
	//draw directional lights
}

void bsRenderQueue::sortLights()
{
	//Lights that are not clipping with the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> notClippingWithCamera;
	//Lights that are clipping with the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> clippingWithCamera;
	//Lights that completely contain the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> containingCamera;

	
	const XMVECTOR& cameraPosition = mCamera->getEntity()->getTransform().getPosition();
	const float nearClip = mCamera->getProjectionInfo().mNearClip;
	const float nearClipSquared = nearClip * nearClip;

	XMVECTOR lightPosition;
	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	//Light position - camera position.
	XMVECTOR deltaPosition;

	for (size_t i = 0; i < mPointLightPositionPairs.size(); ++i)
	{
		const bsLight* light = mPointLightPositionPairs[i].first;
		const XMFLOAT3& position = mPointLightPositionPairs[i].second;

		//Create the light's transform, which includes the light's radius as scaling factor
		//and the entity's position.
		const float scale = light->getLightData().radius;
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslation(position.x, position.y, position.z));
		fullTransform = XMMatrixTranspose(fullTransform);

		XMStoreFloat4x4(&lightWorldTransform, fullTransform);

		lightPosition = XMLoadFloat3(&position);
		deltaPosition = XMVectorSubtract(lightPosition, cameraPosition); 
		
		const float distanceSquared = XMVectorGetX(XMVector3LengthSq(deltaPosition));

		if (distanceSquared > nearClipSquared + (light->getLightData().radius * 2.0f))
		{
			//Light completely outside camera near plane.

			notClippingWithCamera.push_back(std::make_pair(light, lightWorldTransform));
		}
		else if (distanceSquared + nearClipSquared < (light->getLightData().radius * 2.0f))
		{
			//Light completely contains camera near plane.

			containingCamera.push_back(std::make_pair(light, lightWorldTransform));
		}
		else
		{
			//Light clipping with near plane.

			clippingWithCamera.push_back(std::make_pair(light, lightWorldTransform));
		}
	}
}

void bsRenderQueue::drawTexts()
{
	const XMMATRIX viewProjection = mCamera->getViewProjection();

	std::for_each(std::begin(mText3dToDraw), std::end(mText3dToDraw),
		[&](const std::pair<const bsEntity*, const bsText3D*>& text)
	{
		const XMMATRIX& entityTransform = text.first->getTransform().getTransform();
		const XMMATRIX worldTransform = XMMatrixMultiply(
			//180 degrees around X axis
			XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(180.0f)),
			entityTransform);

		XMMATRIX transform = XMMatrixMultiply(worldTransform, viewProjection);
		transform = XMMatrixMultiply(XMMatrixScaling(0.03f, 0.03f, 0.03f), transform);

		text.second->draw(transform);
	});
}

void bsRenderQueue::drawPointLights()
{
	BS_ASSERT2(!mPointLightPositionPairs.empty(), "drawPointLights called, but there are"
		" no point lights to draw");

	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	CBLight cbLight;

	const unsigned int lightCount = mPointLightPositionPairs.size();

	std::vector<LightInstanceData, bsAlignedAllocator<LightInstanceData>> lightData;
	lightData.reserve(lightCount);

	for (size_t i = 0; i < lightCount; ++i)
	{
		const bsLight* light = mPointLightPositionPairs[i].first;
		const XMFLOAT3& position = mPointLightPositionPairs[i].second;

		//Create the light's transform, which includes the light's radius as scaling factor
		//and the entity's position.
		//Multiply with 2 to get diameter rather than radius.
		const float scale = light->getLightData().radius * 2.0f;
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslation(position.x, position.y, position.z));
		fullTransform = XMMatrixTranspose(fullTransform);


		LightInstanceData data;
		data.world = fullTransform;
		const XMFLOAT3& lightColor = light->getLightData().color;
		data.colorIntensity.x = lightColor.x;
		data.colorIntensity.y = lightColor.y;
		data.colorIntensity.z = lightColor.z;
		data.colorIntensity.w = light->getLightData().intensity;

		data.positionRadius.x = position.x;
		data.positionRadius.y = position.y;
		data.positionRadius.z = position.z;
		data.positionRadius.w = light->getLightData().radius;

		//Unused for point lights.
		memset(&data.attenuation, 0, sizeof(data.attenuation));
		memset(&data.direction, 0, sizeof(data.direction));

		lightData.push_back(data);
	}

	drawInstancedLight(*mDx11Renderer, *mPointLightPositionPairs[0].first,
		lightData.data(), lightData.size());
}

void bsRenderQueue::drawSpotLights()
{
	BS_ASSERT2(!mSpotLightPositionPairs.empty(), "drawSpotLights called, but there are"
		" no spot lights to draw");

	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	CBLight cbLight;

	const unsigned int lightCount = mSpotLightPositionPairs.size();

	std::vector<LightInstanceData, bsAlignedAllocator<LightInstanceData>> lightData;
	lightData.reserve(lightCount);

	for (size_t i = 0; i < lightCount; ++i)
	{
		const bsLight* light = mSpotLightPositionPairs[i].first;

		const bsTransform& lightTransform = mSpotLightPositionPairs[i].second->getTransform();
		const XMVECTOR& position = lightTransform.getPosition();
		const XMVECTOR& rotation = lightTransform.getRotation();


		//Create the light's transform, which includes the light's radius as scaling factor
		//and the entity's position.
		//Multiply with 2 to get diameter rather than radius.
		const float scale = light->getLightData().radius * 2.0f;
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslationFromVector(position));
		fullTransform = XMMatrixTranspose(fullTransform);


		LightInstanceData data;
		data.world = fullTransform;
		const XMFLOAT3& lightColor = light->getLightData().color;
		data.colorIntensity.x = lightColor.x;
		data.colorIntensity.y = lightColor.y;
		data.colorIntensity.z = lightColor.z;
		data.colorIntensity.w = light->getLightData().intensity;
		
		XMStoreFloat4A(&data.positionRadius, position);
		data.positionRadius.w = light->getLightData().radius;

		//Rotate the light's direction by its current rotation.
		const XMVECTOR lightDirection = XMVector3Rotate(
			XMLoadFloat3(&light->getLightData().direction), rotation);

		//const XMVECTOR lightDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		XMStoreFloat4A(&data.direction, lightDirection);


		//Unused for spot lights.
		memset(&data.attenuation, 0, sizeof(data.attenuation));

		lightData.push_back(data);
	}

	drawInstancedLight(*mDx11Renderer, *mSpotLightPositionPairs[0].first,
		lightData.data(), lightData.size());
}

size_t bsRenderQueue::MeshRendererHasher::operator()(const bsMeshRenderer* meshRenderer) const
{
	return std::hash<unsigned int>()(meshRenderer->getUniqueID());
}
