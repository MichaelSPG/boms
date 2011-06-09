#include "bsRenderQueue.h"

#include <map>
#include <unordered_map>

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

#include "bsMath.h"
#include "bsConstantBuffers.h"


bsRenderQueue::bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
	: mCamera(nullptr)
	, mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
{
	BS_ASSERT(dx11Renderer);
	BS_ASSERT(shaderManager);


	D3D11_BUFFER_DESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWorld);
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
	D3D11_INPUT_ELEMENT_DESC d;
	ZeroMemory(&d, sizeof(d));
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);
	/*d.SemanticName = "COLOR";
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);
	*/

	mWireframeVertexShader = mShaderManager->getVertexShader("Wireframe.fx", inputLayout);
	mWireframePixelShader = mShaderManager->getPixelShader("Wireframe.fx");


	//Mesh
	inputLayout.clear();
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);

	d.SemanticName = "NORMAL";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 12u;
	inputLayout.push_back(d);

	d.SemanticName = "TEXCOORD";
	d.AlignedByteOffset = 24u;
	d.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(d);

	mMeshVertexShader = mShaderManager->getVertexShader("Mesh.fx", inputLayout);
	mMeshPixelShader = mShaderManager->getPixelShader("Mesh.fx");


	//Light
	inputLayout.clear();
	d.SemanticName = "POSITION";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 0;
	inputLayout.push_back(d);

	mLightVertexShader = mShaderManager->getVertexShader("Light.fx", inputLayout);
	mLightPixelShader = mShaderManager->getPixelShader("Light.fx");
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
	mPrimitivesToDraw.clear();
	mLightsToDraw.clear();
}

void bsRenderQueue::drawGeometry()
{
	mFrameStats.reset();
	bsTimer timer;
	float start = timer.getTimeMilliSeconds(), end;

	unbindGeometryShader();

	sortRenderables();

	//Render all the geometric renderable types
	drawMeshes();
	drawLines();
	drawPrimitives();

	end = timer.getTimeMilliSeconds() - start;
	mFrameStats.timeTakenMs = end;
}

void bsRenderQueue::setWorldConstantBuffer(const XMFLOAT4X4& world)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(1, 1, &mWorldBuffer);
	context->PSSetConstantBuffers(1, 1, &mWorldBuffer);

	context->UpdateSubresource(mWorldBuffer, 0, nullptr, &world, 0, 0);
}

void bsRenderQueue::setWireframeConstantBuffer(const XMFLOAT4X4& world, const XMFLOAT4& color)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(1, 1, &mWireframeWorldBuffer);
	context->PSSetConstantBuffers(1, 1, &mWireframeWorldBuffer);

	CBWireFrame cbWireFrame;
	cbWireFrame.world = world;
	cbWireFrame.color = color;

	context->UpdateSubresource(mWireframeWorldBuffer, 0, nullptr, &cbWireFrame, 0, 0);
}

void bsRenderQueue::setLightConstantBuffer(const CBLight& cbLight)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(2, 1, &mLightBuffer);
	context->PSSetConstantBuffers(2, 1, &mLightBuffer);

	context->UpdateSubresource(mLightBuffer, 0, nullptr, &cbLight, 0, 0);
}

void bsRenderQueue::sortRenderables()
{
	BS_ASSERT2(mCamera, "Camera must be set before attempting to render a frame");

	std::vector<bsSceneNode*> sceneNodes = mCamera->getVisibleSceneNodes();

	//Remove nodes that are marked as not visible.
	sceneNodes.erase(std::remove_if(sceneNodes.begin(), sceneNodes.end(),
		[](const bsSceneNode* sceneNode)
	{
		return !sceneNode->isVisible();
	}), sceneNodes.end());

	mFrameStats.visibleSceneNodeCount = sceneNodes.size();
	if (sceneNodes.empty())
	{
		//Nothing to render
		return;
	}

	//Iterate through all the scene nodes and get the renderables, then sort them into
	//collections depending on their renderable subclass and group up identical renderables
	//that are owned by multiple nodes
	for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
	{
		const auto& renderables = sceneNodes[i]->getRenderables();

		//Iterate through all of this node's renderables and group them
		for (unsigned int j = 0; j < renderables.size(); ++j)
		{
			const bsRenderable::RenderableType identifier = renderables[j]
				->getRenderableType();

			if (identifier == bsRenderable::MESH)
			{
				bsMesh* mesh = static_cast<bsMesh*>(renderables[j].get());
				//See if this mesh already exists in the collection
				auto finder = mMeshesToDraw.find(mesh);

				if (finder == mMeshesToDraw.end())
				{
					//Not found, create it
					std::vector<bsSceneNode*> nodes(1);
					nodes[0] = sceneNodes[i];
					mMeshesToDraw.insert(std::make_pair(mesh, nodes));
				}
				else
				{
					//Found, add the scene node to the mesh' list of scene nodes
					finder->second.push_back(sceneNodes[i]);
				}
			}
			else if (identifier == bsRenderable::WIREFRAME_PRIMITIVE)
			{
				bsPrimitive* primitive = static_cast<bsPrimitive*>(renderables[j].get());
				auto finder = mPrimitivesToDraw.find(primitive);

				if (finder == mPrimitivesToDraw.end())
				{
					//Not found, create it
					std::vector<bsSceneNode*> nodes(1);
					nodes[0] = sceneNodes[i];
					mPrimitivesToDraw.insert(std::make_pair(primitive, nodes));
				}
				else
				{
					//Found
					finder->second.push_back(sceneNodes[i]);
				}
			}
			else if (identifier == bsRenderable::LINE)
			{
				bsLine3D* line = static_cast<bsLine3D*>(renderables[j].get());
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
			else if (identifier == bsRenderable::LIGHT)
			{
				bsLight* light = static_cast<bsLight*>(renderables[j].get());
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
			}
		}
	}
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
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.totalMeshesDrawn += sceneNodes.size();

		//For each scene node which contains this mesh, draw the mesh with different
		//transforms.
		for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
		{
			const hkTransform& transform = sceneNodes[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 xmf4x4(f4x4);
			bsMath::XMFloat4x4Transpose(xmf4x4);
			setWorldConstantBuffer(xmf4x4);

			mesh->draw(mDx11Renderer);
		}
	}
}

void bsRenderQueue::drawLines()
{
	mShaderManager->setPixelShader(mWireframePixelShader);
	mShaderManager->setVertexShader(mWireframeVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	for (auto itr = mLinesToDraw.begin(), end = mLinesToDraw.end(); itr != end; ++itr)
	{
		bsLine3D* currentLine = itr->first;
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.linesDrawn += sceneNodes.size();

		for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
		{
			const hkTransform& transform = sceneNodes[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 world(f4x4);
			bsMath::XMFloat4x4Transpose(world);

			setWireframeConstantBuffer(world, currentLine->mColor);

			currentLine->draw(mDx11Renderer);
		}
	}
}

void bsRenderQueue::drawPrimitives()
{
	//TODO: Make this actually do something
}

void bsRenderQueue::drawLights()
{
	mShaderManager->setPixelShader(mLightPixelShader);
	mShaderManager->setVertexShader(mLightVertexShader);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto itr = mLightsToDraw.begin(), end = mLightsToDraw.end(); itr != end; ++itr)
	{
		const bsLight* currentLight = itr->first;
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.visibleLights += sceneNodes.size();

		for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
		{
			const hkTransform& transform = sceneNodes[i]->getDerivedTransformation();
			float f4x4[16];
			transform.get4x4ColumnMajor(f4x4);
			XMFLOAT4X4 world(f4x4);
			//bsMath::XMFloat4x4Transpose(world);
			//Set scale to equal radius
			//world._44 = 1.0f / currentLight->mRadius;
			//world._44 = 0.3f;
			
			//float scale = 15.0f;
			float scale = currentLight->mRadius;
			/*
			world._11 *= scale;
			world._22 *= scale;
			world._33 *= scale;
			*/
			XMMATRIX m = XMMatrixScaling(scale, scale, scale);
			XMMATRIX origTranslation = XMMatrixIdentity();
			/*
			origTranslation._41 = world._14;
			origTranslation._42 = world._24;
			origTranslation._43 = world._34;
			*/
			
			origTranslation._41 = world._41;
			origTranslation._42 = world._42;
			origTranslation._43 = world._43;
			
			m = XMMatrixMultiply(m, origTranslation);
			XMStoreFloat4x4(&world, m);
			bsMath::XMFloat4x4Transpose(world);

			setWorldConstantBuffer(world);

			CBLight cbLight;
			memcpy(&cbLight.lightColor.x, &currentLight->mColor.x, sizeof(XMFLOAT3));
			cbLight.lightColor.w = currentLight->mIntensity;
			const hkQuadReal& position = sceneNodes[i]->getDerivedPosition().getQuad();
			memcpy(&cbLight.lightPosition.x, &position.x, sizeof(XMFLOAT3));
			cbLight.lightPosition.w = currentLight->mRadius;

			setLightConstantBuffer(cbLight);

			currentLight->draw(mDx11Renderer);
		}
	}
}
