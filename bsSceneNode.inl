#include "bsSceneNode.h"



inline void* bsSceneNode::operator new(size_t)
{
	return _aligned_malloc(sizeof(bsSceneNode), 16);
}

inline void bsSceneNode::operator delete(void* p)
{
	_aligned_free(p);
}

inline const XMVECTOR& bsSceneNode::getPosition() const
{
	return mWorldPosition;
}

inline const XMVECTOR& bsSceneNode::getRotation() const
{
	return mWorldRotation;
}

inline const XMVECTOR& bsSceneNode::getScale() const
{
	return mWorldScale;
}

inline const XMMATRIX& bsSceneNode::getTransposedTransform() const
{
	return mTransposedWorldTransform;
}

inline const XMMATRIX& bsSceneNode::getTransform() const
{
	return mWorldTransform;
}

inline const XMVECTOR& bsSceneNode::getLocalPosition() const
{
	return mLocalPosition;
}

inline const XMVECTOR& bsSceneNode::getLocalRotation() const
{
	return mLocalRotation;
}

inline const XMVECTOR& bsSceneNode::getLocalScale() const
{
	return mLocalScale;
}

inline void bsSceneNode::setLocalPosition(const XMVECTOR& newPosition)
{
	mLocalPosition = newPosition;	

	updateDerivedTransform();
}

inline void bsSceneNode::translate(const XMVECTOR& translation)
{
	const XMVECTOR newPosition = XMVectorAdd(mLocalPosition, translation);
	setLocalPosition(newPosition);
}

inline void bsSceneNode::setLocalRotation(const XMVECTOR& rotation)
{
	mLocalRotation = rotation;

	updateDerivedTransform();

	updateRigidBodyRotation();
}

inline void bsSceneNode::setScaleUniform(float uniformScale)
{
	setScale(XMVectorReplicate(uniformScale));
}

inline void bsSceneNode::setLocalScale(const XMVECTOR& newScale)
{
	mLocalScale = newScale;

	updateDerivedTransform();
}

inline void bsSceneNode::setLocalScaleUniform(float newUniformScale)
{
	setLocalScale(XMVectorReplicate(newUniformScale));
}

inline void bsSceneNode::scale(const XMVECTOR& scaleToMultiply)
{
	setLocalScale(XMVectorMultiply(mLocalScale, scaleToMultiply));
}

inline void bsSceneNode::scaleUniform(float uniformScale)
{
	setLocalScale(XMVectorScale(mLocalScale, uniformScale));
}

inline bsEntity& bsSceneNode::getEntity()
{
	return mEntity;
}

inline const bsEntity& bsSceneNode::getEntity() const
{
	return mEntity;
}

inline bsSceneNode* bsSceneNode::getParentSceneNode() const
{
	return mParentSceneNode;
}

inline const std::vector<bsSceneNode*>& bsSceneNode::getChildren() const
{
	return mChildren;
}
