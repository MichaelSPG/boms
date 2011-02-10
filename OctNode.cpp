#include "OctNode.h"
#include "Log.h"
#include "SceneGraph.h"
#include "Dx11Renderer.h"
#include "AABB.h"

OctNode::OctNode(int id, const XMFLOAT3& position, SceneGraph* sceneGraph,
	OctNode* parent, const AABB& aabb)
	: Node(position, id, sceneGraph)
	, mParent(parent)
	, mAABB(new AABB(aabb))
	, mWireFramePrimitive(nullptr)
{
	/*
	std::ostringstream out;
	out << "Creating a node, ID: " << mID;
	Log::log(out.str().c_str());
	*/

	/*if (!parent)
	{
		mAABB = AABB(0.0f, 0.0f, 0.0f, 1000.0f, 1000.0f, 1000.0f);
	}
	else
	{
		XMVectorGetX(parent->mAABB.mPosition) - XMVectorGetX(parent->mAABB.mHalfExtents);
		
		mAABB = AABB(XMVectorGetX(parent->mAABB.mPosition) - XMVectorGetX(parent->mAABB.mHalfExtents),
					 XMVectorGetY(parent->mAABB.mPosition) - XMVectorGetY(parent->mAABB.mHalfExtents),
					 XMVectorGetZ(parent->mAABB.mPosition) - XMVectorGetZ(parent->mAABB.mHalfExtents),
					 1,1,1);
		
	}*/
}

OctNode::~OctNode()
{
	/*
	std::ostringstream out;
	out << "Deleting a node, ID: " << mID;
	Log::log(out.str().c_str());
	*/

	if (mWireFramePrimitive)
		delete mWireFramePrimitive;
}

void OctNode::createChildren(unsigned short depth, SceneGraph* sceneGraph)
{
	if (--depth == 0)
		return;
	/*
	std::ostringstream out;
	out << "Node with ID " << mID << " making 8 children";
	Log::log(out.str().c_str());
	*/

	mChildren.reserve(8);
	
	//Each child node needs to be exactly half the size of its parent.
	//float halfHalfExtents = mAABB->mHalfExtents.x * 0.5f;

	
	XMFLOAT3 halfHalfExtents = mAABB->mHalfExtents;
	halfHalfExtents.x *= 0.5f;
	halfHalfExtents.y *= 0.5f;
	halfHalfExtents.z *= 0.5f;

	AABB aabb(halfHalfExtents, this);

	/*
	AABB aabb(mAABB->mPosition.x - mAABB->mHalfExtents.x,
			  mAABB->mPosition.y - mAABB->mHalfExtents.y,
			  mAABB->mPosition.z - mAABB->mHalfExtents.z,
			  halfHalfExtents, halfHalfExtents, halfHalfExtents);
	*/

	//Create 8 children and make their AABBs fill the current node's AABB.
	for (int i = 0; i < 8; ++i)
	{
		XMFLOAT3 position;

		if (i % 2 == 1)
		{
			position.x = mPosition.x + halfHalfExtents.x;
		}
		else
		{
			position.x = mPosition.x - halfHalfExtents.x;
		}

		switch (i % 4)
		{
		case 0:
			position.y = mPosition.y + halfHalfExtents.y;
			break;

		case 1:
			position.y = mPosition.y + halfHalfExtents.y;
			break;

		case 2:
			position.z = mPosition.z + halfHalfExtents.z;
			break;

		case 3:
			position.z = mPosition.z - halfHalfExtents.z;
			break;
		}
		
		
		mChildren.push_back(std::make_shared<OctNode>(sceneGraph->getNumCreatedObjects(),
			position, mSceneGraph, this, aabb));
	}

	for (int i = 0; i < 8; ++i)
	{
		mChildren[i]->createChildren(depth, sceneGraph);
	}
}

void OctNode::attachSceneNode(std::shared_ptr<SceneNode> node)
{
	mSceneNodes.push_back(node);
}

void OctNode::drawAABB(Dx11Renderer* dx11Renderer)
{
	assert(dx11Renderer);



	
	mWireFramePrimitive->draw(dx11Renderer);
		//	dx11Renderer->getDeviceContext()->DrawIndexed(36, 0, 0);
	//draw

	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->drawAABB(dx11Renderer);
		}
	}

	
	/*
	Vertex vertices[] =
	{
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f )},
		{ XMFLOAT3(  1.0f, 1.0f, -1.0f )},
		{ XMFLOAT3(  1.0f, 1.0f,  1.0f )},
		{ XMFLOAT3( -1.0f, 1.0f,  1.0f )},

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f )},
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f )},
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f )},
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f )},

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ) },

		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ) },
	};


	/*
	ID3DXLine line;
	//D3DXCreateBox()
	D3DXCreateLine()

	D3DTLVERTEX vertex[4]; // 2 lines
	int x=5,y=60,x2=20,y2=60;
	int x3=22,y3=60,x4=50,y4=60;
	vertex[0]=D3DTLVERTEX(D3DVECTOR(x,y,0.0),1.0,RGB_MAKE(255,255,255),0,0,0);
	vertex[1]=D3DTLVERTEX(D3DVECTOR(x2,y2,0.0),1.0,RGB_MAKE(255,255,255),0,0,0);
	vertex[2]=D3DTLVERTEX(D3DVECTOR(x3,y3,0.0),1.0,RGB_MAKE(255,255,255),0,0,0);
	vertex[3]=D3DTLVERTEX(D3DVECTOR(x4,y4,0.0),1.0,RGB_MAKE(255,255,255),0,0,0);// Then draw it with:lpD3DDevice->DrawPrimitive(D3DPT_LINELIST,D3DFVF_TLVERTEX,&vertex,4,0);            

	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	*/
}

void OctNode::prepareForDrawing(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager)
{
	assert(dx11Renderer);

	mWireFramePrimitive = new Primitive();
	mWireFramePrimitive->createPrimitive(dx11Renderer, shaderManager, *mAABB);

	if (mChildren.size())
	{
		for (int i = 0; i < 8; ++i)
		{
			mChildren[i]->prepareForDrawing(dx11Renderer, shaderManager);
		}
	}
}
