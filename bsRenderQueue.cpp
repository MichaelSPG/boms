#include "bsRenderQueue.h"

#include <assert.h>
#include <map>
#include <unordered_map>

#include "bsSceneNode.h"
#include "bsRenderable.h"
#include "bsMesh.h"
#include "bsPrimitive.h"
#include "bsDx11Renderer.h"
#include "bsShaderManager.h"
#include "bsVertexShader.h"
#include "bsPixelShader.h"
#include "bsLog.h"


bsRenderQueue::bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager)
	: mDx11Renderer(dx11Renderer)
	, mShaderManager(shaderManager)
	, mWorldBufferSet(false)
{
	assert(dx11Renderer);
	assert(shaderManager);


	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));
	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(CBWorld);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0u;
	bufferDescription.MiscFlags = 0u;

	HRESULT hres = mDx11Renderer->getDevice()->CreateBuffer(&bufferDescription, nullptr,
		&mWorldBuffer);

	assert(SUCCEEDED(hres) && "bsRenderQueue::bsRenderQueue failed to create world buffer");
	if (FAILED(hres))
	{
		bsLog::logMessage("bsRenderQueue::bsRenderQueue failed to create world buffer",
			pantheios::SEV_CRITICAL);
	}
}

bsRenderQueue::~bsRenderQueue()
{

}

void bsRenderQueue::reset()
{
	mRenderables.clear();
	mMeshes.clear();
	mWireframePrimitives.clear();
	mWorldBufferSet = false;
}

void bsRenderQueue::addRenderables(const std::vector<std::shared_ptr<bsRenderable>>& renderables)
{
	mRenderables.insert(mRenderables.end(), renderables.cbegin(), renderables.cend());
}

void bsRenderQueue::splitRenderables()
{
	unsigned int count = mRenderables.size();

	for (unsigned int i = 0u; i < count; ++i)
	{
		switch (mRenderables[i]->getRenderableIdentifier())
		{
		case bsRenderable::MESH:
			mMeshes.push_back(static_cast<bsMesh*>(mRenderables[i].get()));
			break;

		case bsRenderable::WIREFRAME_PRIMITIVE:
			mWireframePrimitives.push_back(static_cast<bsPrimitive*>(mRenderables[i].get()));
			break;

		default:
			//If this is reached the switch statement needs to be updated.
			assert(!"bsRenderQueue::sort encountered an unknown renderable");
			break;
		}
	}

	sortMeshes();
}

void bsRenderQueue::sortMeshes()
{
	std::sort(mMeshes.begin(), mMeshes.end());
}

void bsRenderQueue::draw(bsDx11Renderer* dx11Renderer)
{
	//Unbind geometry shader
	setGS(nullptr);
	//splitRenderables();
	sortSceneNodes();

	return;

	std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
	desc.push_back(D3D11_INPUT_ELEMENT_DESC());
	static auto ps = mShaderManager->getPixelShader("HLSL_Basic.fx");
	static auto vs = mShaderManager->getVertexShader("HLSL_Basic.fx", desc);

	mShaderManager->setPixelShader(ps);
	mShaderManager->setVertexShader(vs);

	unsigned int meshCount = mMeshes.size();
	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		drawMesh(mMeshes[i], dx11Renderer->getDeviceContext());
	}
}

void bsRenderQueue::drawMesh(bsMesh* mesh, ID3D11DeviceContext* deviceContext)
{
	mesh->mVertexBuffer;
//	mesh->mSceneNode;

	unsigned int offset = 0u;
	unsigned int stride = sizeof(VertexNormalTex);
	deviceContext->IASetVertexBuffers(0u, 1u, &mesh->mVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(mesh->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0u);
	
	

	deviceContext->DrawIndexed(mesh->mIndices, 0u, 0);
	//mesh->
}

void bsRenderQueue::setWorldConstantBuffer(const XMFLOAT4X4& world)
{
	ID3D11DeviceContext* context = mDx11Renderer->getDeviceContext();

//	if (!mWorldBufferSet)
	{
		context->VSSetConstantBuffers(1u, 1u, &mWorldBuffer);
		context->PSSetConstantBuffers(1u, 1u, &mWorldBuffer);
		mWorldBufferSet = true;
	}

	CBWorld cbWorld;
	cbWorld.world = world;

	context->UpdateSubresource(mWorldBuffer, 0u, nullptr, &cbWorld.world, 0u, 0u);
}

void bsRenderQueue::addSceneNode(bsSceneNode* sceneNode)
{
	mSceneNodes.push_back(sceneNode);
}

void bsRenderQueue::sortSceneNodes()
{
	mRenderablePairs;

	//////////////////////////////////////////////////////////////////////////
	//vector of scene nodes that include each mesh. Probably good for instancing
	std::vector<std::pair<bsMesh*, std::vector<bsSceneNode*>>> yep;
	//////////////////////////////////////////////////////////////////////////

	std::unordered_map<bsMesh*, std::vector<bsSceneNode*>> meshPairs;
	//std::vector<std::pair<bsSceneNode*, bsMesh*>>		meshPairs;
	//std::set<std::pair<bsSceneNode*, bsMesh*>> meshPairs;
	std::vector<std::pair<bsSceneNode*, bsPrimitive*>>	primitivePairs;

	for (unsigned int i = 0u, count = mSceneNodes.size(); i < count; ++i)
	{
		const auto& renderables = mSceneNodes[i]->getRenderables();

		for (unsigned int j = 0u; j < renderables.size(); ++j)
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
					nodes[0] = mSceneNodes[i];
					meshPairs.insert(std::make_pair(mesh, nodes));
				}
				else
				{
					//Found
					finder->second.push_back(mSceneNodes[i]);
				}
				
				//meshPairs.insert(std::make_pair(mSceneNodes[i],
				//	static_cast<bsMesh*>(renderables[j].get())));

				//meshPairs.push_back(std::make_pair(mSceneNodes[i],
				//	static_cast<bsMesh*>(renderables[j].get())));
			}
			else if (identifier == bsRenderable::WIREFRAME_PRIMITIVE)
			{
				primitivePairs.push_back(std::make_pair(mSceneNodes[i],
					static_cast<bsPrimitive*>(renderables[j].get())));
			}
		}
	}
	for (auto itr = meshPairs.begin(); itr != meshPairs.end(); ++itr)
	{
		bsMesh* mesh = itr->first;
		const std::vector<bsSceneNode*>& sceneNodes = itr->second;

		for (unsigned int i = 0u; i < sceneNodes.size(); ++i)
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
	for (unsigned int i = 0u; i < meshPairs.size(); ++i)
	{
		
	}

	mRenderablePairs;


	/*
	bsSceneNode* test;
	

	std::sort(mSceneNodes.begin(), mSceneNodes.end(),
		[](const bsSceneNode* lhs, const bsSceneNode* rhs)
		{
			if (lhs->)
		});
	*/
}

void bsRenderQueue::setVS(bsVertexShader* vs)
{
	if (vs)
	{
		auto context = mDx11Renderer->getDeviceContext();
		context->VSSetShader(vs->getVertexShader(), nullptr, 0);
		context->IASetInputLayout(vs->getInputLayout());
	}
	else
	{
		mDx11Renderer->getDeviceContext()->VSSetShader(nullptr, nullptr, 0);
	}
}

void bsRenderQueue::setPS(bsPixelShader* ps)
{
	mDx11Renderer->getDeviceContext()->PSSetShader(ps->getPixelShader(), nullptr, 0);
}

void bsRenderQueue::setGS(void* gs)
{
	mDx11Renderer->getDeviceContext()->GSSetShader(nullptr, nullptr, 0u);
}

//Draw primitive

/*
context->VSSetConstantBuffers(1, 1, &mBuffer);
context->PSSetConstantBuffers(1, 1, &mBuffer);


context->DrawIndexed(24, 0, 0);
*/