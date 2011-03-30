#include "bsNode.h"

#include <assert.h>

#include "bsPrimitive.h"
#include "bsShaderManager.h"
#include "bsLog.h"


bsNode::bsNode(const hkVector4& translation, int id, bsSceneGraph* sceneGraph)
	: mAABB(hkVector4(0.0f, 0.0f, 0.0f, 0.0f), hkVector4(0.0f, 0.0f, 0.0f, 0.0f))
	, mTransform(hkRotation(), hkVector4(0.0f, 0.0f, 0.0f, 0.0f))
	, mID(id)
	, mSceneGraph(sceneGraph)
	, mWireFramePrimitive(nullptr)
{
	setTranslation(translation);
	mTransform.getRotation().setIdentity();
}

bsNode::~bsNode()
{
	if (mWireFramePrimitive)
	{
		delete mWireFramePrimitive;
	}
}

void bsNode::drawAABB(bsDx11Renderer* dx11Renderer) const
{
	assert(dx11Renderer);
	assert(mWireFramePrimitive && mWireFramePrimitive->mBuffer
		&& "createDrawableAabb must be called before drawAABB");
	if (!(mWireFramePrimitive && mWireFramePrimitive->mBuffer))
	{
		bsLog::logMessage("bsNode::drawAABB: Trying to draw an AABB that has not been created",
			pantheios::SEV_CRITICAL);

		return;
	}

	CBWireFrame cb;
	XMStoreFloat4x4(&cb.world, XMMatrixIdentity());
	const hkVector4& translation = mTransform.getTranslation();
	cb.world._14 = translation.getSimdAt(0);
	cb.world._24 = translation.getSimdAt(1);
	cb.world._34 = translation.getSimdAt(2);
	cb.color = XMFLOAT4(mWireframeColor.x, mWireframeColor.y, mWireframeColor.z, 0.0f);
	/*
	dx11Renderer->getDeviceContext()->UpdateSubresource(mWireFramePrimitive->mBuffer, 0,
		nullptr, &cb, 0, 0);
*/

	mWireFramePrimitive->draw(dx11Renderer);
}

void bsNode::createDrawableAabb(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager,
	bool octNode /*= true*/)
{
	assert(dx11Renderer);
	if (mWireFramePrimitive)
	{
		delete mWireFramePrimitive;
	}
	mWireFramePrimitive = new bsPrimitive();
	mWireFramePrimitive->createPrimitive(dx11Renderer, shaderManager, mAABB, octNode);
}

void bsNode::setTranslation(const float x, const float y, const float z)
{
	hkVector4 translation(mTransform.getTranslation());

	mTransform.getTranslation().set(x, y, z, 0.0f);

	//Get translation
	translation.sub3clobberW(mTransform.getTranslation());

	//Translate the AABB
	mAABB.m_min.add3clobberW(translation);
	mAABB.m_max.add3clobberW(translation);

	verifyTranslation();
}
