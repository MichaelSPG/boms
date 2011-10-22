#include "StdAfx.h"

#include "bsRenderQueue.h"

#include <unordered_map>
#include <algorithm>

#include "bsCamera.h"
#include "bsSceneNode.h"
#include "bsRenderable.h"
#include "bsMesh.h"
#include "bsPrimitive.h"
#include "bsLine3D.h"
#include "bsLight.h"

#include "bsDx11Renderer.h"
#include "bsShaderManager.h"
#include "bsVertexShader.h"
#include "bsPixelShader.h"
#include "bsLog.h"
#include "bsTimer.h"
#include "bsAssert.h"
#include "bsText3D.h"

#include "bsMath.h"
#include "bsConstantBuffers.h"

#include "bsAlignedAllocator.h"


bsRenderQueue::bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
	: mCamera(nullptr)
	, mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
	, mUseInstancing(false)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(shaderManager);


	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(XMMATRIX);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;
	bufferDescription.MiscFlags = 0;

	HRESULT hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mWorldBuffer);

	BS_ASSERT2(SUCCEEDED(hres), "bsRenderQueue::bsRenderQueue failed to create world buffer");

	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mWireframeWorldBuffer);
	BS_ASSERT(SUCCEEDED(hres));

	bufferDescription.ByteWidth = sizeof(CBLight);
	hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mLightBuffer);
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

	mWireframeVertexShader = mShaderManager->getVertexShader("Wireframe.fx", inputLayout.data(), inputLayout.size());
	mWireframePixelShader = mShaderManager->getPixelShader("Wireframe.fx");


	D3D11_INPUT_ELEMENT_DESC layout[7] =
	{
		//Vertex data.
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		//Instance data (4x4 matrix).
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	mMeshInstancedVertexShader = mShaderManager->getVertexShader("MeshInstanced.fx", layout, 7);
	mMeshInstancedPixelShader = mShaderManager->getPixelShader("MeshInstanced.fx");

	{
		D3D11_INPUT_ELEMENT_DESC layoutNotInstanced[3] =
		{
			//Vertex data.
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		mMeshVertexShader = mShaderManager->getVertexShader("Mesh.fx", layoutNotInstanced, 3);
		mMeshPixelShader = mShaderManager->getPixelShader("MeshInstanced.fx");
	}

	//Light
	inputLayout.clear();
	inputElementDesc.SemanticName = "POSITION";
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc.AlignedByteOffset = 0;
	inputLayout.push_back(inputElementDesc);

	mLightVertexShader = mShaderManager->getVertexShader("Light.fx", inputLayout.data(), inputLayout.size());
	mLightPixelShader = mShaderManager->getPixelShader("Light.fx");

	D3D11_INPUT_ELEMENT_DESC lightInstanced[7] =
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
	};

	mLightInstancedVertexShader = mShaderManager->getVertexShader("LightInstanced.fx", lightInstanced, 7);
	mLightInstancedPixelShader = mShaderManager->getPixelShader("LightInstanced.fx");
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
	mLightPositionPairs.clear();
	mText3dToDraw.clear();
}

void bsRenderQueue::drawGeometry()
{
	mFrameStats.reset();

	unbindGeometryShader();

	sortRenderables();
	sortLights();

	//Render all the geometric renderable types
	if (mUseInstancing)
	{
		drawMeshesInstanced();
	}
	else
	{
		drawMeshes();
	}
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

void bsRenderQueue::sortRenderables()
{
	BS_ASSERT2(mCamera, "Camera must be set before attempting to render a frame");

	std::vector<bsSceneNode*> sceneNodes = mCamera->getVisibleSceneNodes();

	mFrameStats.visibleSceneNodeCount = sceneNodes.size();
	

	//Gather all the renderables from the visible nodes.
	for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
	{
		bsEntity& entity = sceneNodes[i]->getEntity();

		const std::shared_ptr<bsMesh>& mesh = entity.getComponent<std::shared_ptr<bsMesh>&>();
		if (mesh)
		{
			auto finder = mMeshesToDraw.find(mesh.get());

			if (finder == mMeshesToDraw.end())
			{
				//Not found, create it
				std::vector<bsSceneNode*> nodes(1);
				nodes[0] = sceneNodes[i];
				mMeshesToDraw.insert(std::make_pair(mesh.get(), nodes));
			}
			else
			{
				//Found, add the scene node to the mesh' list of scene nodes
				finder->second.push_back(sceneNodes[i]);
			}
		}

		bsLine3D* line = entity.getComponent<bsLine3D*>();
		if (line != nullptr)
		{
			auto finder = mLinesToDraw.find(line);

			if (finder == mLinesToDraw.end())
			{
				//Not found, create it
				std::vector<bsSceneNode*> nodes(1);
				nodes[0] = sceneNodes[i];
				mLinesToDraw.insert(std::make_pair(line, nodes));
			}
			else
			{
				//Found
				finder->second.push_back(sceneNodes[i]);
			}
		}

		bsLight* light = entity.getComponent<bsLight*>();
		if (light != nullptr)
		{
			XMFLOAT3 position;
			XMStoreFloat3(&position, sceneNodes[i]->getPosition());
			mLightPositionPairs.push_back(std::make_pair(light, position));

#if 0
			auto finder = mLightsToDraw.find(light);

			if (finder == mLightsToDraw.end())
			{
				//Not found, create it
				std::vector<bsSceneNode*> nodes(1);
				nodes[0] = sceneNodes[i];
				mLightsToDraw.insert(std::make_pair(light, nodes));
			}
			else
			{
				//Found
				finder->second.push_back(sceneNodes[i]);
			}
#endif
		}

		bsText3D* text = entity.getComponent<bsText3D*>();
		if (text != nullptr)
		{
			mText3dToDraw.push_back(std::make_pair(sceneNodes[i], text));
		}
	}

	mFrameStats.visibleLights = mLightPositionPairs.size();
}

void bsRenderQueue::unbindGeometryShader()
{
	mDx11Renderer->getDeviceContext()->GSSetShader(nullptr, nullptr, 0);
}

void bsRenderQueue::drawMeshes()
{
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShaderManager->setPixelShader(mMeshPixelShader);
	mShaderManager->setVertexShader(mMeshVertexShader);

	mFrameStats.uniqueMeshesDrawn = mMeshesToDraw.size();

	for (auto itr = mMeshesToDraw.begin(), end = mMeshesToDraw.end(); itr != end; ++itr)
	{
		bsMesh* mesh = itr->first;
		if (!mesh->hasFinishedLoading())
		{
			continue;
		}
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.totalMeshesDrawn += sceneNodes.size();

		//For each scene node which contains this mesh, draw the mesh with different
		//transforms.
		for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
		{
			/*
			const hkTransform& transform = sceneNodes[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 xmf4x4(f4x4);
			bsMath::XMFloat4x4Transpose(xmf4x4);
			setWorldConstantBuffer(xmf4x4);

			mesh->draw(mDx11Renderer);
			*/


			/*
			const XMVECTOR& pos = sceneNodes[i]->getPosition();
			const XMVECTOR& rot = sceneNodes[i]->getDerivedRotation2();
			const XMVECTOR& scale = sceneNodes[i]->getDerivedScale2();

			const XMMATRIX positionMat = XMMatrixTranslationFromVector(pos);
			const XMMATRIX rotationMat = XMMatrixRotationQuaternion(rot);
			const XMMATRIX scaleMat = XMMatrixScalingFromVector(scale);

			XMMATRIX transformMat = XMMatrixMultiply(scaleMat, rotationMat);
			transformMat = XMMatrixMultiply(transformMat, positionMat);
			transformMat = XMMatrixTranspose(transformMat);
			*/
			//XMFLOAT4X4 xmf4x4;
			//XMStoreFloat4x4(&xmf4x4, transformMat);
			//XMStoreFloat4x4(&xmf4x4, );
			setWorldConstantBuffer(sceneNodes[i]->getTransposedTransform());

			mesh->draw(mDx11Renderer);

			const unsigned int triangleCount = mesh->getTriangleCount();
			mFrameStats.totalTrianglesDrawn += triangleCount;
			mFrameStats.totalTrianglesDrawnNotInstanced += triangleCount;
		}
	}
}

void drawMeshInstanced(const bsDx11Renderer& renderer, const bsMesh& mesh,
	const XMMATRIX* transforms, unsigned int transformCount);

void bsRenderQueue::drawMeshesInstanced()
{
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShaderManager->setPixelShader(mMeshInstancedPixelShader);
	mShaderManager->setVertexShader(mMeshInstancedVertexShader);

	mFrameStats.uniqueMeshesDrawn = mMeshesToDraw.size();


	std::vector<XMMATRIX, bsAlignedAllocator<XMMATRIX>> transforms;

	for (auto itr = mMeshesToDraw.begin(), end = mMeshesToDraw.end(); itr != end; ++itr)
	{
		const bsMesh& mesh = *itr->first;
		if (!mesh.hasFinishedLoading())
		{
			continue;
		}

		const std::vector<bsSceneNode*>& sceneNodes = itr->second;
		mFrameStats.totalMeshesDrawn += sceneNodes.size();
		mFrameStats.totalTrianglesDrawn += mesh.getTriangleCount();
		mFrameStats.totalTrianglesDrawnNotInstanced += mesh.getTriangleCount() * sceneNodes.size();

		transforms.clear();
		transforms.reserve(sceneNodes.size());
		for (unsigned int i = 0; i < sceneNodes.size(); ++i)
		{
			transforms.push_back(sceneNodes[i]->getTransposedTransform());
		}

		drawMeshInstanced(*mDx11Renderer, mesh, transforms.data(), transforms.size());
	}
}

void drawMeshInstanced(const bsDx11Renderer& renderer, const bsMesh& mesh,
	const XMMATRIX* transforms, unsigned int transformCount)
{
	D3D11_BUFFER_DESC instanceBufferDesc = { 0 };
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(XMMATRIX) * transformCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA instanceData = { 0 };
	instanceData.pSysMem = transforms;

	ID3D11Buffer* instanceBuffer;
	HRESULT hr = renderer.getDevice()->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
	BS_ASSERT(SUCCEEDED(hr));

	mesh.drawInstanced(*renderer.getDeviceContext(), instanceBuffer, transformCount);


	instanceBuffer->Release();
}

void bsRenderQueue::drawLines()
{
	mShaderManager->setPixelShader(mWireframePixelShader);
	mShaderManager->setVertexShader(mWireframeVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		//D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto itr = mLinesToDraw.begin(), end = mLinesToDraw.end(); itr != end; ++itr)
	{
		bsLine3D* currentLine = itr->first;
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.linesDrawn += sceneNodes.size();

		for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
		{
			/*
			const hkTransform& transform = sceneNodes[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 world(f4x4);
			bsMath::XMFloat4x4Transpose(world);
			*/
			setWireframeConstantBuffer(sceneNodes[i]->getTransposedTransform(), currentLine->mColor);

			currentLine->draw(mDx11Renderer);
		}
	}
}

void bsRenderQueue::drawLights()
{
	if (mUseInstancing)
	{
		drawLightsInstanced();
	}
	else
	{
		drawLightsNotInstanced();
	}
}

void drawInstancedLight(const bsDx11Renderer& renderer, bsLight& light,
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

	//bsLight::drawInstanced(*renderer.getDeviceContext(), instanceBuffer, instanceCount);
	light.drawInstanced(*renderer.getDeviceContext(), instanceBuffer, instanceCount);

	instanceBuffer->Release();
}

void bsRenderQueue::drawLightsInstanced()
{
	mShaderManager->setPixelShader(mLightInstancedPixelShader);
	mShaderManager->setVertexShader(mLightInstancedVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	CBLight cbLight;

	const unsigned int lightCount = mLightPositionPairs.size();

	std::vector<LightInstanceData, bsAlignedAllocator<LightInstanceData>> lightData;
	lightData.reserve(lightCount);

	for (size_t i = 0; i < lightCount; ++i)
	{
		const bsLight* light = mLightPositionPairs[i].first;
		const XMFLOAT3& position = mLightPositionPairs[i].second;

		//Create the light's transform, which includes the light's radius as scaling factor
		//and the node's position.
		//Multiply with 2 to get diameter rather than radius.
		const float scale = light->getRadius() * 2.0f;
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslation(position.x, position.y, position.z));
		fullTransform = XMMatrixTranspose(fullTransform);


		LightInstanceData data;
		data.world = fullTransform;
		data.colorIntensity = XMVectorSet(light->mColor.x, light->mColor.y, light->mColor.z,
			light->mIntensity);
		data.positionRadius = XMVectorSet(position.x, position.y, position.z, light->mRadius);
		lightData.push_back(data);


		//XMStoreFloat4x4(&lightWorldTransform, fullTransform);
#if 0
		setWorldConstantBuffer(fullTransform);


		memcpy(&cbLight.lightColor.x, &light->mColor.x, sizeof(XMFLOAT3));
		cbLight.lightColor.w = light->mIntensity;

		memcpy(&cbLight.lightPosition.x, &position.x, sizeof(XMFLOAT3));
		cbLight.lightPosition.w = light->mRadius;

		setLightConstantBuffer(cbLight);

		light->draw(mDx11Renderer);
#endif
	}

	drawInstancedLight(*mDx11Renderer, *mLightPositionPairs[0].first,
		lightData.data(), lightData.size());
	//
	//for (unsigned int i = 0; i < lightCount; ++i)
	//{
	//	drawInstancedLight(*mDx11Renderer, *mLightPositionPairs[i].first,
	//		lightData.data(), lightData.size());
	//}
}

void bsRenderQueue::drawLightsNotInstanced()
{
	mShaderManager->setPixelShader(mLightPixelShader);
	mShaderManager->setVertexShader(mLightVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	CBLight cbLight;

	for (size_t i = 0; i < mLightPositionPairs.size(); ++i)
	{
		const bsLight* light = mLightPositionPairs[i].first;
		const XMFLOAT3& position = mLightPositionPairs[i].second;

		//Create the light's transform, which includes the light's radius as scaling factor
		//and the node's position.
		//Multiply with 2 to get diameter rather than radius.
		const float scale = light->getRadius() * 2.0f;
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslation(position.x, position.y, position.z));
		fullTransform = XMMatrixTranspose(fullTransform);

		//XMStoreFloat4x4(&lightWorldTransform, fullTransform);

		setWorldConstantBuffer(fullTransform);


		memcpy(&cbLight.lightColor.x, &light->mColor.x, sizeof(XMFLOAT3));
		cbLight.lightColor.w = light->mIntensity;

		memcpy(&cbLight.lightPosition.x, &position.x, sizeof(XMFLOAT3));
		cbLight.lightPosition.w = light->mRadius;
		
		setLightConstantBuffer(cbLight);

		light->draw(mDx11Renderer);
	}
}

void bsRenderQueue::sortLights()
{
	//Lights that are not clipping with the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> notClippingWithCamera;
	//Lights that are clipping with the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> clippingWithCamera;
	//Lights that completely contain the camera near plane.
	std::vector<std::pair<const bsLight*, XMFLOAT4X4>> containingCamera;

	
	const XMVECTOR& cameraPosition = mCamera->getEntity()->getOwner()->getPosition();
	const float nearClip = mCamera->getProjectionInfo().mNearClip;
	const float nearClipSquared = nearClip * nearClip;

	XMVECTOR lightPosition;
	XMMATRIX scalingMatrix;
	XMMATRIX fullTransform;
	XMFLOAT4X4 lightWorldTransform;
	//Light position - camera position.
	XMVECTOR deltaPosition;

	for (size_t i = 0; i < mLightPositionPairs.size(); ++i)
	{
		const bsLight* light = mLightPositionPairs[i].first;
		const XMFLOAT3& position = mLightPositionPairs[i].second;

		//Create the light's transform, which includes the light's radius as scaling factor
		//and the node's position.
		const float scale = light->getRadius();
		scalingMatrix = XMMatrixScaling(scale, scale, scale);
		fullTransform = XMMatrixMultiply(scalingMatrix,
			XMMatrixTranslation(position.x, position.y, position.z));
		fullTransform = XMMatrixTranspose(fullTransform);

		XMStoreFloat4x4(&lightWorldTransform, fullTransform);

		lightPosition = XMLoadFloat3(&position);
		deltaPosition = XMVectorSubtract(lightPosition, cameraPosition); 
		
		const float distanceSquared = XMVectorGetX(XMVector3LengthSq(deltaPosition));

		if (distanceSquared > nearClipSquared + (light->getRadius() * 2.0f))
		{
			//Light completely outside camera near plane.

			notClippingWithCamera.push_back(std::make_pair(light, lightWorldTransform));
		}
		else if (distanceSquared + nearClipSquared < (light->getRadius() * 2.0f))
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
		[&](const std::pair<bsSceneNode*, bsText3D*>& text)
	{
		const XMMATRIX& nodeTransform = text.first->getTransform();
		const XMMATRIX worldTransform = XMMatrixMultiply(
			//180 degrees around X axis
			XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(180.0f)),
			nodeTransform);

		XMMATRIX transform = XMMatrixMultiply(worldTransform, viewProjection);
		transform = XMMatrixMultiply(XMMatrixScaling(0.03f, 0.03f, 0.03f), transform);

		text.second->draw(transform);
	});
}
