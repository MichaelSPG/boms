#pragma once

#include <string>
#include <unordered_map>

#include <windows.h>

#include <OIS.h>

#include "bsRenderStats.h"
#include "bsPrimitiveCreator.h"

class bsLog;
class bsCore;
class bsDeferredRenderer;
class bsMesh;
class bsText2D;
class bsSceneNode;
class bsScene;


class Application : public OIS::KeyListener, public OIS::MouseListener
{
public:
	Application(HINSTANCE hInstance, int showCmd, const int windowWidth, const int windowHeight);

	~Application();

	void update(float deltaTime);

	virtual bool keyPressed(const OIS::KeyEvent& arg);

	virtual bool keyReleased(const OIS::KeyEvent& arg);

	virtual bool mouseMoved(const OIS::MouseEvent& arg);

	virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

	virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

	//Returns true if the application should quit.
	inline bool quit() const
	{
		return mQuit;
	}


private:
	void createSomeLights();

	void createMeshes();

	void createTexts();

	void createNodes();

	void createSpheres(unsigned int count);

	void createBoxes(bool staticBoxes, unsigned int count);

	void createWalls(float offsetFromCenter, float length, float height);

	void createKeyframedRb();

	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	bool w, a, s, d, space, c, shift, rightMouseDown, leftMouseDown, mQuit, pause;
	float mCameraSpeed;

	bsCore*		mCore;

	bsDeferredRenderer*	mDeferredRenderer;

	std::unordered_map<std::string, std::shared_ptr<bsMesh>> mMeshes;
	std::unordered_map<std::string, std::shared_ptr<bsText2D>> mTexts;
	std::unordered_map<std::string, bsSceneNode*> nodes;

	bsRenderStats mRenderStats;
	bsScene* mScene;
	bsPrimitiveCreator* mPrimCreator;
};
