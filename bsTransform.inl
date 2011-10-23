#include "bsTransform.h"


inline void* bsTransform::operator new(size_t)
{
	//Need to use this to make sure SSE types are properly aligned.
	return _aligned_malloc(sizeof(bsTransform), 16);
}

inline void bsTransform::operator delete(void* p)
{
	_aligned_free(p);
}

inline const XMVECTOR& bsTransform::getPosition() const
{
	return mWorldPosition;
}

inline const XMVECTOR& bsTransform::getRotation() const
{
	return mWorldRotation;
}

inline const XMVECTOR& bsTransform::getScale() const
{
	return mWorldScale;
}

inline const XMMATRIX& bsTransform::getTransposedTransform() const
{
	return mTransposedWorldTransform;
}

inline const XMMATRIX& bsTransform::getTransform() const
{
	return mWorldTransform;
}

inline const XMVECTOR& bsTransform::getLocalPosition() const
{
	return mLocalPosition;
}

inline const XMVECTOR& bsTransform::getLocalRotation() const
{
	return mLocalRotation;
}

inline const XMVECTOR& bsTransform::getLocalScale() const
{
	return mLocalScale;
}

inline void bsTransform::setLocalPosition(const XMVECTOR& newPosition)
{
	mLocalPosition = newPosition;	

	updateDerivedTransform();
}

inline void bsTransform::translate(const XMVECTOR& translation)
{
	//Calculate new local position.
	const XMVECTOR newPosition = XMVectorAdd(mLocalPosition, translation);
	setLocalPosition(newPosition);
}

inline void bsTransform::setLocalRotation(const XMVECTOR& rotation)
{
	mLocalRotation = rotation;

	updateDerivedTransform();

	updateRigidBodyRotation();
}

inline void bsTransform::setScaleUniform(float uniformScale)
{
	setScale(XMVectorReplicate(uniformScale));
}

inline void bsTransform::setLocalScale(const XMVECTOR& newScale)
{
	mLocalScale = newScale;

	updateDerivedTransform();
}

inline void bsTransform::setLocalScaleUniform(float newUniformScale)
{
	setLocalScale(XMVectorReplicate(newUniformScale));
}

inline void bsTransform::scale(const XMVECTOR& scaleToMultiply)
{
	setLocalScale(XMVectorMultiply(mLocalScale, scaleToMultiply));
}

inline void bsTransform::scaleUniform(float uniformScale)
{
	setLocalScale(XMVectorScale(mLocalScale, uniformScale));
}

inline bsEntity& bsTransform::getEntity()
{
	return *mEntity;
}

inline const bsEntity& bsTransform::getEntity() const
{
	return *mEntity;
}

inline const bsTransform* bsTransform::getParentTransform() const
{
	return mParentTransform;
}

inline const std::vector<bsTransform*>& bsTransform::getChildren() const
{
	return mChildren;
}
