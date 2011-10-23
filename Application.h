#pragma once

#include <string>
#include <unordered_map>

#include <windows.h>

#include <OIS.h>

#include "bsRenderStats.h"
#include "bsPrimitiveCreator.h"

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/Constraint/Bilateral/PointToPath/hkpLinearParametricCurve.h>

class bsLog;
class bsCore;
class bsDeferredRenderer;
class bsMesh;
class bsText2D;
class bsEntity;
class bsScene;
class bsCharacterController;


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

	void createEntities();

	void createSpheres(unsigned int count);

	void createBoxes(bool staticBoxes, unsigned int count);

	void createWalls(float offsetFromCenter, float length, float height);

	void createKeyframedRb();

	void createLines();

	void createFactory();
	void createFactoryLights();
	bsEntity* createLightAtPosition(const XMVECTOR& position, float radius, const XMFLOAT3& color);

	void toggleFreeCam();

	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	bool w, a, s, d, space, c, shift, rightMouseDown, leftMouseDown, mQuit;
	float mCameraSpeed;

	bsCore*		mCore;

	bsDeferredRenderer*	mDeferredRenderer;

	std::unordered_map<std::string, std::shared_ptr<bsMesh>> mMeshes;
	std::unordered_map<std::string, std::shared_ptr<bsText2D>> mTexts;
	std::unordered_map<std::string, bsEntity*> entities;

	bsRenderStats mRenderStats;
	bsScene* mScene;
	bsPrimitiveCreator* mPrimCreator;
	
	bsCharacterController* mCharacterController;
	bsEntity* characterControllerEntity;
	bool mFreeCamMode;

	hkpLinearParametricCurve* mCurve;
};
