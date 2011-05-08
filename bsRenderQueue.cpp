#include "bsRenderQueue.h"

#include <cassert>
#include <map>
#include <unordered_map>

#include "bsCamera.h"
#include "bsSceneNode.h"
#include "bsRenderable.h"
#include "bsMesh.h"
#include "bsPrimitive.h"
#include "bsLine3D.h"

#include "bsDx11Renderer.h"
#include "bsShaderManager.h"
#include "bsVertexShader.h"
#include "bsPixelShader.h"
#include "bsLog.h"
#include "bsTimer.h"

#include "bsMath.h"
#include "bsConstantBuffers.h"


bsRenderQueue::bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
	: mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
{
	assert(dx11Renderer);
	assert(shaderManager);


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWorld);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;
	bufferDescription.MiscFlags = 0;

	HRESULT hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mWorldBuffer);

	assert(SUCCEEDED(hres) && "bsRenderQueue::bsRenderQueue failed to create world buffer");
	if (FAILED(hres))
	{
		bsLog::logMessage("bsRenderQueue::bsRenderQueue failed to create world buffer",
			pantheios::SEV_CRITICAL);
	}

	bufferDescription.ByteWidth = sizeof(CBWireFrame);
	hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr, &mWireframeWorldBuffer);
	assert(SUCCEEDED(hres));

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

	mMeshVertexShader = mShaderManager->getVertexShader("HLSL_Basic.fx", inputLayout);
	mMeshPixelShader = mShaderManager->getPixelShader("HLSL_Basic.fx");
}

bsRenderQueue::~bsRenderQueue()
{
	mWorldBuffer->Release();
	mWireframeWorldBuffer->Release();
}

void bsRenderQueue::reset()
{
	mFrameStats.reset();
}

void bsRenderQueue::draw()
{
	mFrameStats.reset();
	bsTimer timer;
	float start = timer.getTimeMilliSeconds(), end;

	unbindGeometryShader();

	sortSceneNodes();

	end = timer.getTimeMilliSeconds() - start;
	mFrameStats.timeTakenMs = end;
}

void bsRenderQueue::setWorldConstantBuffer(const XMFLOAT4X4& world)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	context->VSSetConstantBuffers(1, 1, &mWorldBuffer);
	context->PSSetConstantBuffers(1, 1, &mWorldBuffer);

	CBWorld cbWorld;
	cbWorld.world = world;

	context->UpdateSubresource(mWorldBuffer, 0, nullptr, &cbWorld.world, 0, 0);
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

void bsRenderQueue::sortSceneNodes()
{
	std::unordered_map<bsMesh*, std::vector<bsSceneNode*>>		meshPairs;
	std::unordered_map<bsLine3D*, std::vector<bsSceneNode*>>	lines;
	std::vector<std::pair<bsSceneNode*, bsPrimitive*>>			primitivePairs;

	const std::vector<bsSceneNode*>& sceneNodes = mCamera->getVisibleSceneNodes();

	mFrameStats.visibleSceneNodeCount = sceneNodes.size();
	if (sceneNodes.empty())
	{
		//Nothing to render
		return;
	}

	for (unsigned int i = 0, count = sceneNodes.size(); i < count; ++i)
	{
		const auto& renderables = sceneNodes[i]->getRenderables();

		for (unsigned int j = 0; j < renderables.size(); ++j)
		{
			bsRenderable::RenderableIdentifier identifier = renderables[j]
				->getRenderableIdentifier();

			if (identifier == bsRenderable::MESH)
			{
				auto mesh = static_cast<bsMesh*>(renderables[j].get());
				auto finder = meshPairs.find(mesh);

				if (finder == meshPairs.end())
				{
					//Not found, create it
					std::vector<bsSceneNode*> nodes(1);
					nodes[0] = sceneNodes[i];
					meshPairs.insert(std::make_pair(mesh, nodes));
				}
				else
				{
					//Found
					finder->second.push_back(sceneNodes[i]);
				}
			}
			else if (identifier == bsRenderable::WIREFRAME_PRIMITIVE)
			{
				primitivePairs.push_back(std::make_pair(sceneNodes[i],
					static_cast<bsPrimitive*>(renderables[j].get())));
			}
			else if (identifier == bsRenderable::LINES)
			{
				bsLine3D* line = static_cast<bsLine3D*>(renderables[j].get());
				auto finder = lines.find(line);

				if (finder == lines.end())
				{
					//Not found, create it
					std::vector<bsSceneNode*> nodes(1);
					nodes[0] = sceneNodes[i];
					lines.insert(std::make_pair(line, nodes));
				}
				else
				{
					//Found
					finder->second.push_back(sceneNodes[i]);
				}
			}
		}
	}

	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

	//Meshes
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShaderManager->setPixelShader(mMeshPixelShader);
	mShaderManager->setVertexShader(mMeshVertexShader);

	mFrameStats.uniqueMeshesDrawn = meshPairs.size();

	for (auto itr = meshPairs.begin(), end = meshPairs.end(); itr != end; ++itr)
	{
		bsMesh* mesh = itr->first;
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		mFrameStats.totalMeshesDrawn += sceneNodes.size();

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
	

	//Lines

	mShaderManager->setPixelShader(mWireframePixelShader);
	mShaderManager->setVertexShader(mWireframeVertexShader);

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	for (auto itr = lines.begin(), end = lines.end(); itr != end; ++itr)
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

void bsRenderQueue::unbindGeometryShader()
{
	mDx11Renderer->getDeviceContext()->GSSetShader(nullptr, nullptr, 0);
}
