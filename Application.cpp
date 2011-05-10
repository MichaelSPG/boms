#include "Application.h"

#include "bsConfig.h"

#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>

#include "bsLog.h"
#include "bsSceneGraph.h"
#include "bsCamera.h"
#include "bsRenderStats.h"
#include "bsRenderQueue.h"
#include "bsTextBox.h"
#include "bsTextManager.h"
#include "bsStringUtils.h"
#include "bsWindow.h"
#include "bsHavokManager.h"
#include "bsCore.h"
#include "bsSceneNode.h"
#include "bsLine3D.h"
#include "bsResourceManager.h"
#include "bsLight.h"

#include "bsDeferredRenderer.h"


Application::Application(HINSTANCE hInstance, int showCmd, const int windowWidth,
	const int windowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
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

	float clearColor[4] = {0.0f, 0.0f, 0.15f, 0.0f};
	mCore->getDx11Renderer()->setRenderTargetClearColor(clearColor);

	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	//Register the text manager with the rendering system so that it'll draw texts
	mDeferredRenderer->registerEndOfRenderCallback(boost::bind(&bsTextManager::drawAllTexts, textManager));

	//////////////////////////////////////////////////////////////////////////
	

	bsLog::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);
}

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	bsCamera* camera = mCore->getSceneGraph()->getCamera();

	if (w)
	{
		camera->translate(0.0f, 0.0f, shift ? 6.0f : 1.2f);
	}
	if (s)
	{
		camera->translate(0.0f, 0.0f, shift ? -6.0f : -1.2f);
	}
	if (a)
	{
		camera->translate(shift ? -6.0f : -1.2f, 0.0f, 0.0f);
	}
	if (d)
	{
		camera->translate(shift ? 6.0f : 1.2f, 0.0f, 0.0f);
	}
	if (space)
	{
		camera->translate(0.0f, shift ? 6.0f : 1.2f, 0.0f);
	}
	if (c)
	{
		camera->translate(0.0f, shift ? -6.0f : -1.2f, 0.0f);
	}
	camera->update();
	

	static std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);

	d.SemanticName = "NORMAL";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);

	d.SemanticName = "TEXCOORD";
	d.AlignedByteOffset = 24;
	d.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(d);

	bsResourceManager* resourceManager = mCore->getResourceManager();


	static auto context = mCore->getDx11Renderer()->getDeviceContext();
	static bsMeshManager* meshManager = resourceManager->getMeshManager();

	static auto roi = meshManager->getMesh("roi.bsm");
	static auto duck = meshManager->getMesh("duck.bsm");
	static auto teapot = meshManager->getMesh("teapot.bsm");
	static auto gourd = meshManager->getMesh("gourd.bsm");
	static auto greeble = meshManager->getMesh("greeble.bsm");
	static auto sphere = meshManager->getMesh("sphere_1m_d.bsm");

	static auto cubeWithOffset = meshManager->getMesh("cubeWithOffset.bsm");

	bsSceneGraph* sceneGraph = mCore->getSceneGraph();
	static bsSceneNode* identityNode = sceneGraph->createSceneNode();

	static bsSceneNode* testNode1 = sceneGraph->createSceneNode(hkVector4(-70.0f, 0.0f, 0.0f));

	static bsSceneNode* derivedNode1 = testNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode2 = derivedNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode3 = derivedNode2->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	

	static std::vector<bsSceneNode*> derivedNodes;
	bsSceneNode*& previousNode = derivedNode1;

	static bool derNodesOnce = false;
	if (!derNodesOnce)
	{
		identityNode->attachRenderable(sphere);

		for (unsigned int i = 0; i < 10; ++i)
		{
			derivedNodes.push_back(previousNode->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f)));
			previousNode = derivedNodes.back();
		}

		derNodesOnce = true;
	}

	static std::shared_ptr<bsLine3D> line3d = std::make_shared<bsLine3D>(XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f));

	static bool once = false;
	if (!once)
	{
		bool disable = false;
		if (!disable)
		{
			bsPointLightCInfo ci;
			ci.color = XMFLOAT3(1.0f, 1.0f, 0.0f);
			ci.intensity = 1.0f;
			ci.radius = 25.0f;
			std::shared_ptr<bsLight> light = std::make_shared<bsLight>(bsLight::LT_POINT, meshManager, ci);

			for (unsigned int i = 0; i < derivedNodes.size(); ++i)
			{
				derivedNodes[i]->attachRenderable((i & 1) == 0 ? duck : greeble);
				derivedNodes[i]->attachRenderable(light);
			}

			identityNode->attachRenderable(light);
			identityNode->translate(hkVector4(10.0f, -5.0f, 0.0f));
		}
		
		once = true;
	}

	static hkVector4 unit(0.0f, 1.0f, 0.0f);
	static float increaseForever = 0.0f;
	increaseForever += pause ? 0.0f : 0.01f;

	
	for (unsigned int i = 0; i < derivedNodes.size(); ++i)
	{
		derivedNodes[i]->setPosition(hkVector4(sinf(increaseForever) * 100.0f, 0.0f, 0.0f));
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

	static auto statsText = textManager->createText(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);
	statsText->setText(stats.getStatsString());

	static std::shared_ptr<bsTextBox> textBox = textManager->createTextBox(4500.0f, 10);
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);

	static auto frameStats = textManager->createText(L"");
	frameStats->setPosition(2.5f, 450.0f);
	frameStats->addFlags(FW1_RESTORESTATE);
	frameStats->setColor(0xff00ff00);
	frameStats->setFontSize(14.0f);
	
	frameStats->setText(mDeferredRenderer->getRenderQueue()->getFrameStats().getFrameStatsStringWide());
	
	//Add callback for the log so messages will also be shown on the screen during runtime
	static bool logCallBackOnce = false;
	if (!logCallBackOnce)
	{
		bsTextBox* tb = textBox.get();
		bsLog::addCallback(boost::bind(&bsTextBox::addTextLine, tb, _1));
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
