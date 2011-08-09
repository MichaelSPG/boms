#include "Application.h"

#include "bsConfig.h"

#include <algorithm>

#include <boost/bind.hpp>

#include <vld.h>

#include "bsLog.h"
#include "bsSceneGraph.h"
#include "bsCamera.h"
#include "bsRenderStats.h"
#include "bsRenderQueue.h"
#include "bsScrollingText2D.h"
#include "bsTextManager.h"
#include "bsStringUtils.h"
#include "bsWindow.h"
#include "bsHavokManager.h"
#include "bsCore.h"
#include "bsSceneNode.h"
#include "bsLine3D.h"
#include "bsResourceManager.h"
#include "bsLight.h"
#include "bsDx11Renderer.h"

#include "bsDeferredRenderer.h"

#include "bsAssert.h"


Application::Application(HINSTANCE hInstance, int showCmd, const int windowWidth,
	const int windowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mCameraSpeed(1.0f)
{
	w = a = s = d = space = c = shift = false;
	rightMouseDown = leftMouseDown = mQuit = pause = false;

	bsCoreCInfo coreCInfo;
	coreCInfo.hInstance = hInstance;
	coreCInfo.showCmd = showCmd;
	mCore = new bsCore(coreCInfo);

	bsLog::logMessage("Initializing OIS");

	//OIS initialization
	OIS::ParamList paramList;
	paramList.insert(OIS::ParamList::value_type("WINDOW",
		bsStringUtils::toString((unsigned long)mCore->getWindow()->getHwnd())));
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND"))); 
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE"))); 
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NOWINKEY")));

	mInputManager = OIS::InputManager::createInputSystem(paramList);

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

	mMouse->getMouseState().width = windowWidth;
	mMouse->getMouseState().height = windowHeight;

	mKeyboard->setEventCallback(this);
	mMouse->setEventCallback(this);

	/*
	RECT rect;
	GetWindowRect(mCore->getWindow()->getHwnd(), &rect);
	int x = rect.right - rect.left;
	int y = rect.bottom - rect.top;
	*/

	bsWindow* window = mCore->getWindow();
	
	mDeferredRenderer = new bsDeferredRenderer(mCore->getDx11Renderer(),
		mCore->getSceneGraph()->getCamera(), mCore->getResourceManager()->getShaderManager(),
		window);

	mCore->setRenderSystem(mDeferredRenderer);

	float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	mCore->getDx11Renderer()->setRenderTargetClearColor(clearColor);

	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	//Register the text manager with the rendering system so that it'll draw texts
	mDeferredRenderer->registerEndOfRenderCallback(boost::bind(&bsTextManager::drawAllTexts, textManager));
	
	//////////////////////////////////////////////////////////////////////////

	bsLog::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);

	createSomeLights();
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);

	delete mCore;
	delete mDeferredRenderer;
}

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	bsCamera* camera = mCore->getSceneGraph()->getCamera();

	//Movement + speedup if shift is pressed
	
#if 0
	{
		const hkTransform& cameraTransform = camera->getTransform2();
		hkVector4 translation(0.0f, 0.0f, 0.0f);
		translation(0) = (a ? (-mCameraSpeed) : (d ? mCameraSpeed : 0.0f));
		translation(1) = (s ? (-mCameraSpeed) : (w ? mCameraSpeed : 0.0f));
		translation(2) = (c ? (-mCameraSpeed) : (space ? mCameraSpeed : 0.0f));
		
		if (shift)
		{
			translation.mul4(2.5f);
		}
		
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
#endif
	
	if (w)
	{
		//camera->translate(hkVector4(0.0f, 0.0f, shift ? mCameraSpeed * 2.5f : mCameraSpeed));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(0.0f, 0.0f, shift ? mCameraSpeed * 2.5f : mCameraSpeed);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	if (s)
	{
		//camera->translate(hkVector4(0.0f, 0.0f, shift ? -mCameraSpeed * 2.5f : -mCameraSpeed));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(0.0f, 0.0f, shift ? -mCameraSpeed * 2.5f : -mCameraSpeed);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	if (a)
	{
		//camera->translate(hkVector4(shift ? -mCameraSpeed * 2.5f : -mCameraSpeed, 0.0f, 0.0f));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(shift ? -mCameraSpeed * 2.5f : -mCameraSpeed, 0.0f, 0.0f);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	if (d)
	{
		//camera->translate(hkVector4(shift ? mCameraSpeed * 2.5f : mCameraSpeed, 0.0f, 0.0f));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(shift ? mCameraSpeed * 2.5f : mCameraSpeed, 0.0f, 0.0f);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	if (space)
	{
		//camera->translate(hkVector4(0.0f, shift ? mCameraSpeed * 2.5f : mCameraSpeed, 0.0f));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(0.0f, shift ? mCameraSpeed * 2.5f : mCameraSpeed, 0.0f);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	if (c)
	{
		//camera->translate(hkVector4(0.0f, shift ? -mCameraSpeed * 2.5f : -mCameraSpeed, 0.0f));
		const hkTransform& transform = camera->getTransform2();
		hkVector4 translation(0.0f, shift ? -mCameraSpeed * 2.5f : -mCameraSpeed, 0.0f);
		translation.setMul3(transform.getRotation(), translation);
		camera->translate(translation);
	}
	
	camera->update();
	

	static bsResourceManager* resourceManager = mCore->getResourceManager();

	static auto context = mCore->getDx11Renderer()->getDeviceContext();
	static bsMeshCache* meshCache(resourceManager->getMeshCache());
	
	static std::shared_ptr<bsMesh> roi(meshCache->getMesh("roi.bsm"));
	static std::shared_ptr<bsMesh> duck(meshCache->getMesh("duck.bsm"));
	static std::shared_ptr<bsMesh> teapot(meshCache->getMesh("teapot.bsm"));
	static std::shared_ptr<bsMesh> gourd(meshCache->getMesh("gourd.bsm"));
	static std::shared_ptr<bsMesh> greeble(meshCache->getMesh("greeble.bsm"));
	static std::shared_ptr<bsMesh> sphere(meshCache->getMesh("sphere_1m_d.bsm"));

	static std::shared_ptr<bsMesh> cubeWithOffset(meshCache->getMesh("cubeWithOffset.bsm"));

	static bsSceneGraph* sceneGraph = mCore->getSceneGraph();
	static bsSceneNode* identityNode = sceneGraph->createSceneNode();

	static bsSceneNode* testNode1 = sceneGraph->createSceneNode(hkVector4(-70.0f, 0.0f, 0.0f));

	static bsSceneNode* derivedNode1 = testNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode2 = derivedNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode3 = derivedNode2->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));

	static bsSceneNode* greebleTownNode = sceneGraph->createSceneNode();
	static std::shared_ptr<bsMesh> greebleTown(meshCache->getMesh("greeble_town_small.bsm"));

	//static std::shared_ptr<bsMesh> greebleTownBig(meshCache->getMesh("greeble_town.bsm"));

	static bool greebleOnce = false;
	if (!greebleOnce)
	{
		greebleTownNode->attachRenderable(greebleTown);
		//greebleTownNode->attachRenderable(greebleTownBig);
		greebleOnce = true;
	}
	

	static std::vector<bsSceneNode*> derivedNodes;
	static std::vector<bsSceneNode*> moreDerivedNodes;
	static bsSceneNode*& previousNode = derivedNode1;

	static bool derNodesOnce = true;
	if (!derNodesOnce)
	{
		//Create node hierarchy
		//identityNode->attachRenderable(sphere);

		for (unsigned int i = 0; i < 10; ++i)
		{
			derivedNodes.push_back(previousNode->createChildSceneNode(hkVector4(25.0f, 0.0f, 0.0f)));
			moreDerivedNodes.push_back(derivedNodes[i]->createChildSceneNode(hkVector4(0.0f, 15.0f, 0.0f)));

			previousNode = derivedNodes.back();
		}

		derNodesOnce = true;
	}

	static std::shared_ptr<bsLine3D> line3d(std::make_shared<bsLine3D>(XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f)));

#if 0
	static bool once = false;
	if (!once)
	{
		bool disable = false;
		if (!disable)
		{
			bsPointLightCInfo ci;
			ci.color = XMFLOAT3(1.0f, 1.0f, 0.0f);
			ci.intensity = 1.0f;
			ci.radius = 10.0f;
			std::shared_ptr<bsLight> light(std::make_shared<bsLight>(bsLight::LT_POINT, meshCache, ci));

			for (unsigned int i = 0; i < derivedNodes.size(); ++i)
			{
				//derivedNodes[i]->attachRenderable((i & 1) == 0 ? duck : greeble);
				//moreDerivedNodes[i]->attachRenderable(light);
			}

			//identityNode->attachRenderable(light);
			//identityNode->translate(hkVector4(10.0f, -5.0f, 0.0f));
		}
		
		once = true;
	}
#endif

	const static hkVector4 unit(0.0f, 1.0f, 0.0f);
	static float increaseForever = 0.0f;
	increaseForever += (pause ? 0.0f : 0.01f);

	
	for (unsigned int i = 0; i < derivedNodes.size(); ++i)
	{
		derivedNodes[i]->setPosition(hkVector4(sinf(increaseForever) * 5.0f, 0.0f, 0.0f));
		if ((i & 3) == 0)
		{
			continue;
		}

		derivedNodes[i]->setRotation(hkQuaternion(unit, (float)(increaseForever * (i * 0.2f))));
	}
	
	static bsTextManager* textManager = resourceManager->getTextManager();

	textManager->updateTexts(deltaTime);

	static bsRenderStats stats;
	stats.setFps(1.0f / deltaTime * 1000.0f);
	stats.setFrameTime(deltaTime);

	static auto statsText = textManager->createText2D(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);
	statsText->setText(stats.getStatsString());

	static std::shared_ptr<bsScrollingText2D> textBox(textManager->createScrollingText2D(4500.0f, 10));
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);

	static auto frameStats = textManager->createText2D(L"");
	frameStats->setPosition(2.5f, 450.0f);
	frameStats->addFlags(FW1_RESTORESTATE);
	frameStats->setColor(0xff00ff00);
	frameStats->setFontSize(14.0f);
	frameStats->setText(mDeferredRenderer->getRenderQueue()->getFrameStats().getFrameStatsStringWide());
	
	//Add callback for the log so messages will also be shown on the screen during runtime
	static bool logCallBackOnce = false;
	if (!logCallBackOnce)
	{
		bsScrollingText2D* tb = textBox.get();
		bsLog::addCallback(boost::bind(&bsScrollingText2D::addTextLine, tb, _1));
		logCallBackOnce = true;
	}


	if (!mCore->update(deltaTime))
	{
		mQuit = true;
	}
}

bool Application::keyPressed(const OIS::KeyEvent& arg)
{
	if (arg.key == OIS::KC_W)
	{
		w = true;
	}
	if (arg.key == OIS::KC_S)
	{
		s = true;
	}
	if (arg.key == OIS::KC_D)
	{
		d = true;
	}
	if (arg.key == OIS::KC_A)
	{
		a = true;
	}
	if (arg.key == OIS::KC_SPACE)
	{
		space = true;
	}
	if (arg.key == OIS::KC_C)
	{
		c = true;
	}
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = true;
	}

	switch (arg.key)
	{
	case OIS::KC_ADD:
		{
			auto pi = mCore->getSceneGraph()->getCamera()->getProjectionInfo();
			pi.mFieldOfView += 0.25f;
			mCore->getSceneGraph()->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mCore->getSceneGraph()->getCamera()->getProjectionInfo();
			pi.mFieldOfView -= 0.25f;
			mCore->getSceneGraph()->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_X:
		mCore->getSceneGraph()->getCamera()->rotateAboutAxis(hkVector4(0.0f, 1.0f, 0.0f), 22.5f);
		break;

	case OIS::KC_V:
		mCore->getDx11Renderer()->setVsyncEnabled(!mCore->getDx11Renderer()->getVsyncEnabled());
		break;

	case OIS::KC_P:
		pause = !pause;
		break;

	case OIS::KC_1:
		mCameraSpeed = 0.25f;
		break;

	case OIS::KC_2:
		mCameraSpeed = 0.5f;
		break;

	case OIS::KC_3:
		mCameraSpeed = 1.0f;
		break;

	case OIS::KC_4:
		mCameraSpeed = 2.0f;
		break;

	case OIS::KC_5:
		mCameraSpeed = 4.0f;
		break;

	case OIS::KC_T:
		mCore->getResourceManager()->getTextManager()->toggleVisibility();
		break;

	}


	return true;
}

bool Application::keyReleased(const OIS::KeyEvent& arg)
{
	if (arg.key == OIS::KC_W)
	{
		w = false;
	}
	if (arg.key == OIS::KC_S)
	{
		s = false;
	}
	if (arg.key == OIS::KC_D)
	{
		d = false;
	}
	if (arg.key == OIS::KC_A)
	{
		a = false;
	}
	if (arg.key == OIS::KC_SPACE)
	{
		space = false;
	}
	if (arg.key == OIS::KC_C)
	{
		c = false;
	}
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = false;
	}

	return true;
}

bool Application::mouseMoved(const OIS::MouseEvent& arg)
{
	if (leftMouseDown)
	{
		bsCamera* camera = mCore->getSceneGraph()->getCamera();

		camera->rotateX(-(float)arg.state.X.rel * 0.01f);
		camera->rotateY(-(float)arg.state.Y.rel * 0.01f);
	}
	
	return true;
}

bool Application::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	switch (id)
	{
	case OIS::MB_Left:
		leftMouseDown = true;
		break;

	case OIS::MB_Right:
		rightMouseDown = true;
		break;
	}
	
	return true;
}

bool Application::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	switch (id)
	{
	case OIS::MB_Left:
		leftMouseDown = false;
		break;

	case OIS::MB_Right:
		rightMouseDown = false;
		break;
	}

	return true;
}

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>

void Application::createSomeLights()
{
	std::vector<bsSceneNode*> nodes(5);
	XMFLOAT3 lights[5] = 
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < nodes.size(); ++i)
	{
		bsPointLightCInfo ci;
		ci.color = lights[i];
		ci.intensity = 1.0f;
		//ci.radius = 10.0f;
		ci.radius = 50.0f;
		std::shared_ptr<bsLight> light(std::make_shared<bsLight>(bsLight::LT_POINT,
			mCore->getResourceManager()->getMeshCache(), ci));

		nodes[i] = mCore->getSceneGraph()->createSceneNode();
		nodes[i]->attachRenderable(light);
	}
	//auto torus = mCore->getResourceManager()->getMeshCache()->getMesh("torus_knot.bsm");
	//nodes[0]->attachRenderable(torus);


	nodes[0]->setPosition(hkVector4(0.0f, 2.5f, 0.0f));
	nodes[1]->setPosition(hkVector4(0.0f, 2.5f, 25.0f));
	nodes[2]->setPosition(hkVector4(0.0f, 2.5f, -25.0f));
	nodes[3]->setPosition(hkVector4(25.0f, 2.5f, 0.0f));
	nodes[4]->setPosition(hkVector4(-25.0f, 2.5f, 0.0f));

#if 0
	{
		hkPseudoRandomGenerator rng(GetTickCount());
		const float worldSize = 250.0f;
		const hkVector4 minRange(-worldSize, 1.0f, -worldSize);
		const hkVector4 maxRange(worldSize, 10.0f, worldSize);
		hkVector4 out;

		bsPointLightCInfo lightCi;
		lightCi.intensity = 1.0f;


		std::vector<bsSceneNode*> sceneNodes(500);
		for (unsigned int i = 0; i < sceneNodes.size(); ++i)
		{
			//Random position
			rng.getRandomVectorRange(minRange, maxRange, out);
			out(1) = 5.0f;
			sceneNodes[i] = mCore->getSceneGraph()->createSceneNode(out);

			//Random color
			rng.getRandomVector01(out);
			memcpy(&lightCi.color, &out(0), 12);

			//Random radius
			lightCi.radius = rng.getRandRange(25, 75.0f);

			std::shared_ptr<bsLight> light(std::make_shared<bsLight>(bsLight::LT_POINT,
				mCore->getResourceManager()->getMeshCache(), lightCi));

			sceneNodes[i]->attachRenderable(light);
		}
	}
#endif

	auto bp = static_cast<hkpHybridBroadPhase*>(mCore->getHavokManager()->getGraphicsWorld()->getBroadPhase());
	bp->fullOptimize();
}
