#include "RenderQueue.h"

#include <assert.h>
#include <map>
#include <unordered_map>

#include "SceneNode.h"
#include "Renderable.h"
#include "Mesh.h"
#include "Primitive.h"
#include "Dx11Renderer.h"
#include "ShaderManager.h"
#include "VertexShader.h"
#include "PixelShader.h"


RenderQueue::RenderQueue(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager)
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

	assert(SUCCEEDED(hres) && "RenderQueue::RenderQueue failed to create world buffer");
	if (FAILED(hres))
	{
		Log::logMessage("RenderQueue::RenderQueue failed to create world buffer",
			pantheios::SEV_CRITICAL);
	}
}

RenderQueue::~RenderQueue()
{

}

void RenderQueue::reset()
{
	mRenderables.clear();
	mMeshes.clear();
	mWireframePrimitives.clear();
	mWorldBufferSet = false;
}

void RenderQueue::addRenderables(const std::vector<std::shared_ptr<Renderable>>& renderables)
{
	mRenderables.insert(mRenderables.end(), renderables.cbegin(), renderables.cend());
}

void RenderQueue::splitRenderables()
{
	unsigned int count = mRenderables.size();

	for (unsigned int i = 0u; i < count; ++i)
	{
		switch (mRenderables[i]->getRenderableIdentifier())
		{
		case Renderable::MESH:
			mMeshes.push_back(static_cast<Mesh*>(mRenderables[i].get()));
			break;

		case Renderable::WIREFRAME_PRIMITIVE:
			mWireframePrimitives.push_back(static_cast<Primitive*>(mRenderables[i].get()));
			break;

		default:
			//If this is reached the switch statement needs to be updated.
			assert(!"RenderQueue::sort encountered an unknown renderable");
			break;
		}
	}

	sortMeshes();
}

void RenderQueue::sortMeshes()
{
	std::sort(mMeshes.begin(), mMeshes.end());
}

void RenderQueue::draw(Dx11Renderer* dx11Renderer)
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

void RenderQueue::drawMesh(Mesh* mesh, ID3D11DeviceContext* deviceContext)
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

void RenderQueue::setWorldConstantBuffer(const XMFLOAT4X4& world)
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

void RenderQueue::addSceneNode(SceneNode* sceneNode)
{
	mSceneNodes.push_back(sceneNode);
}

void RenderQueue::sortSceneNodes()
{
	mRenderablePairs;

	//////////////////////////////////////////////////////////////////////////
	//vector of scene nodes that include each mesh. Probably good for instancing
	std::vector<std::pair<Mesh*, std::vector<SceneNode*>>> yep;
	//////////////////////////////////////////////////////////////////////////

	std::unordered_map<Mesh*, std::vector<SceneNode*>> meshPairs;
	//std::vector<std::pair<SceneNode*, Mesh*>>		meshPairs;
	//std::set<std::pair<SceneNode*, Mesh*>> meshPairs;
	std::vector<std::pair<SceneNode*, Primitive*>>	primitivePairs;

	for (unsigned int i = 0u, count = mSceneNodes.size(); i < count; ++i)
	{
		const auto& renderables = mSceneNodes[i]->getRenderables();

		for (unsigned int j = 0u; j < renderables.size(); ++j)
		{
			Renderable::RenderableIdentifier identifier = renderables[j]
				->getRenderableIdentifier();

			if (identifier == Renderable::MESH)
			{
				auto mesh = static_cast<Mesh*>(renderables[j].get());
				auto finder = meshPairs.find(mesh);

				if (finder == meshPairs.end())
				{
					//Not found, create it
					std::vector<SceneNode*> nodes(1);
					nodes[0] = mSceneNodes[i];
					meshPairs.insert(std::make_pair(mesh, nodes));
				}
				else
				{
					//Found
					finder->second.push_back(mSceneNodes[i]);
				}
				
				//meshPairs.insert(std::make_pair(mSceneNodes[i],
				//	static_cast<Mesh*>(renderables[j].get())));

				//meshPairs.push_back(std::make_pair(mSceneNodes[i],
				//	static_cast<Mesh*>(renderables[j].get())));
			}
			else if (identifier == Renderable::WIREFRAME_PRIMITIVE)
			{
				primitivePairs.push_back(std::make_pair(mSceneNodes[i],
					static_cast<Primitive*>(renderables[j].get())));
			}
		}
	}
	for (auto itr = meshPairs.begin(); itr != meshPairs.end(); ++itr)
	{
		Mesh* mesh = itr->first;
		const std::vector<SceneNode*>& sceneNodes = itr->second;

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
	SceneNode* test;
	

	std::sort(mSceneNodes.begin(), mSceneNodes.end(),
		[](const SceneNode* lhs, const SceneNode* rhs)
		{
			if (lhs->)
		});
	*/
}

void RenderQueue::setVS(VertexShader* vs)
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

void RenderQueue::setPS(PixelShader* ps)
{
	mDx11Renderer->getDeviceContext()->PSSetShader(ps->getPixelShader(), nullptr, 0);
}

void RenderQueue::setGS(void* gs)
{
	mDx11Renderer->getDeviceContext()->GSSetShader(nullptr, nullptr, 0u);
}

//Draw primitive

/*
context->VSSetConstantBuffers(1, 1, &mBuffer);
context->PSSetConstantBuffers(1, 1, &mBuffer);


context->DrawIndexed(24, 0, 0);
*/