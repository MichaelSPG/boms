#ifndef BS_CORE_H
#define BS_CORE_H

#include "bsConfig.h"
#include "bsCoreCInfo.h"

class bsSceneGraph;
class bsDx11Renderer;
class bsResourceManager;
class bsHavokManager;
class bsWindow;
class bsRenderQueue;


class bsCore
{
public:
	bsCore(const bsCoreCInfo& cInfo);

	~bsCore();

	//Updates all the component of the engine.
	//Returns false if the program should exit (Alt+F4, close button, etc).
	bool update(const float deltaTimeMs);

	void preRender();
	
	inline bsSceneGraph* getSceneGraph() const
	{
		return mSceneGraph;
	}

	inline bsResourceManager* getResourceManager() const
	{
		return mResourceManager;
	}

	inline bsHavokManager* getHavokManager() const
	{
		return mHavokManager;
	}

	inline bsDx11Renderer* getDx11Renderer() const
	{
		return mDx11Renderer;
	}

	inline bsWindow* getWindow() const
	{
		return mWindow;
	}

	inline bsRenderQueue* getRenderQueue() const
	{
		return mRenderQueue;
	}

private:
	//Non-copyable.
	bsCore(const bsCore&);
	void operator=(const bsCore&);


	bsWindow*			mWindow;
	bsDx11Renderer*		mDx11Renderer;
	bsSceneGraph*		mSceneGraph;
	bsResourceManager*	mResourceManager;
	bsHavokManager*		mHavokManager;
	bsRenderQueue*		mRenderQueue;

	bsCoreCInfo			mCInfo;
};

#endif // BS_CORE_H