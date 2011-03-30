#include "Application.h"

#include "bsConfig.h"

#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>
//#include <boost/bind/mem_fn.hpp>

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>

#include "bsLog.h"
#include "bsTimer.h"
#include "bsSceneGraph.h"
#include "bsCamera.h"
#include "bsRenderQueue.h"
#include "bsRenderStats.h"
#include "bsColorUtil.h"
#include "bsTextBox.h"
#include "bsTextManager.h"
#include "bsStringUtils.h"
#include "bsWindow.h"


Application::Application(bsWindow* window)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mDx11Renderer(nullptr)
	, mResourceManager(nullptr)
	, mSceneGraph(nullptr)
	, mCamera(nullptr)
	, mWindow(window)
{
	w = a = s = d = space = c = shift = moveDuck = resetDuck = false;
	rightMouseDown = leftMouseDown = mQuit = false;

	bsLog::init(pantheios::SEV_DEBUG);
	bsLog::logMessage("Initializing application", pantheios::SEV_NOTICE);

	mDx11Renderer = new bsDx11Renderer(mWindow->getHwnd(), mWindow->getWindowWidth(),
		mWindow->getWindowHeight());

	//OIS
	{
		bsLog::logMessage("Initializing OIS");

		OIS::ParamList paramList;;
		
		paramList.insert(OIS::ParamList::value_type("WINDOW",
			bsStringUtils::toString(unsigned long(mWindow->getHwnd()))));
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND"))); 
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE"))); 
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NOWINKEY")));

		mInputManager = OIS::InputManager::createInputSystem(paramList);

		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
		
		mMouse->getMouseState().height = mWindow->getWindowHeight();
		mMouse->getMouseState().width = mWindow->getWindowWidth();

		mKeyboard->setEventCallback(this);
		mMouse->setEventCallback(this);
	}

	mResourceManager = new bsResourceManager();
	mResourceManager->initAll("../assets", mDx11Renderer);

	mSceneGraph = new bsSceneGraph(4, mDx11Renderer, mResourceManager);

	mCamera = new bsCamera(bsProjectionInfo(45.0f, 1000.0f, 0.1f, 1280.0f/720.0f), mSceneGraph);

	bsLog::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);

	if (mWindow)
	{
		delete mWindow;
	}
	if (mCamera)
	{
		delete mCamera;
	}
	if (mSceneGraph)
	{
		delete mSceneGraph;
	}
	if (mResourceManager)
	{
		delete mResourceManager;
	}
	if (mDx11Renderer)
	{
		delete mDx11Renderer;
	}
}

void Application::update(float deltaTime)
{
	if (!mWindow->checkForMessages())
	{
		mQuit = true;
		return;
	}

	mDx11Renderer->preRender();

	mKeyboard->capture();
	mMouse->capture();
	
	if (w)
	{
		mCamera->translate(0.0f, 0.0f, shift ? 6.0f : 1.2f);
	}
	if (s)
	{
		mCamera->translate(0.0f, 0.0f, shift ? -6.0f : -1.2f);
	}
	if (a)
	{
		mCamera->translate(shift ? -6.0f : -1.2f, 0.0f, 0.0f);
	}
	if (d)
	{
		mCamera->translate(shift ? 6.0f : 1.2f, 0.0f, 0.0f);
	}
	if (space)
	{
		mCamera->translate(0.0f, shift ? 6.0f : 1.2f, 0.0f);
	}
	if (c)
	{
		mCamera->translate(0.0f, shift ? -6.0f : -1.2f, 0.0f);
	}

	mCamera->update();

	
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	
//	mSceneGraph->getRootNode()->drawAABB(mDx11Renderer);


	static std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0u;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0u;
	d.AlignedByteOffset = 0u;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0u;
	inputLayout.push_back(d);

	d.SemanticName = "NORMAL";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 12u;
	inputLayout.push_back(d);

	d.SemanticName = "TEXCOORD";
	d.AlignedByteOffset = 24u;
	d.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(d);

	static auto vs = mResourceManager->getShaderManager()->getVertexShader("HLSL_Basic.fx", inputLayout);
	static auto ps = mResourceManager->getShaderManager()->getPixelShader("HLSL_Basic.fx");

	//////////////////////////////////////////////////////////////////////////
	static auto context = mDx11Renderer->getDeviceContext();

	static auto roi = mResourceManager->getMeshManager()->getMesh("roi.bsm");
	static auto duck = mResourceManager->getMeshManager()->getMesh("duck.bsm");
	static auto teapot = mResourceManager->getMeshManager()->getMesh("teapot.bsm");
	static auto gourd = mResourceManager->getMeshManager()->getMesh("gourd.bsm");


	static bsSceneNode* testNode1 = mSceneGraph->createSceneNode(hkVector4(0.0f, 0.0f, 0.0f));
	static bsSceneNode* testNode2 = mSceneGraph->createSceneNode(hkVector4(50.0f, 0.0f, 50.0f));
	static bsSceneNode* testNode3 = mSceneGraph->createSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* testNode4 = mSceneGraph->createSceneNode(hkVector4(0.0f, 0.0f, -50.0f));

	static bool once = false;
	if (!once)
	{
		testNode1->attachRenderable(duck);
		bool disable = false;
		if (!disable)
		{
			testNode1->attachRenderable(roi);
			testNode2->attachRenderable(teapot);
			testNode2->attachRenderable(gourd);
			testNode2->attachRenderable(roi);
			testNode3->attachRenderable(duck);
			testNode4->attachRenderable(gourd);
			testNode4->attachRenderable(duck);
			testNode4->attachRenderable(teapot);
			testNode4->attachRenderable(gourd);
		}
		
		once = true;
	}

	static float wave = 0.0f;
	wave += 0.02f;
	float waveSin = sinf(wave);
	float waveCos = cosf(wave);
	//float mulFactor = 1.2f;
	float mulFactor = 1.0f;

	if (moveDuck)
	{
		testNode1->setTranslation( waveSin * mulFactor,  waveCos * mulFactor, -waveSin * mulFactor);
	}
	if (resetDuck)
	{
		auto& dt = const_cast<hkVector4&>(testNode1->getDerivedTranslation());
		dt.setAll3(0.0f);
		testNode1->setTranslation(0.0f, 0.0f, 0.0f);
	}

//	testNode1->setTranslation( waveSin * mulFactor,  waveCos * mulFactor, -waveSin * mulFactor);
	testNode2->setTranslation(-waveSin * mulFactor, -waveCos * mulFactor,  waveSin * mulFactor);
	testNode3->setTranslation( waveCos * mulFactor,  waveSin * mulFactor, -waveCos * mulFactor);
	testNode4->setTranslation( waveSin * mulFactor, -waveSin * mulFactor,  waveCos * mulFactor);

	static float rotWave = 0.0f;
	rotWave += 0.1f;
	waveSin = sinf(rotWave);
	waveCos = cosf(rotWave);
	hkQuaternion rot(hkVector4(0.0f, 1.0f, 0.0f), waveSin);
	rot.mul(hkQuaternion(hkVector4(1.0f, 0.0f, 0.0f), waveCos));
	//rot.mul(hkQuaternion(hkVector4(0.0f, 0.0f, 1.0f), -waveSin));
	//testNode4->setRotation(rot);
	static hkVector4 unit(0.0f, 1.0f, 0.0f);
	hkVector4 angle(waveSin, waveCos, -waveSin);
	angle.normalize3();
	unit.addMul4(0.1f, angle);
	unit.normalize3();
	hkQuaternion q2(unit, waveSin * 0.1f);
	testNode4->setRotation(q2);
	/*
	testNode1->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode2->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode3->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode4->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	
	const hkTransform& trans1 = testNode1->getDerivedTransformation();
	const hkTransform& trans2 = testNode2->getDerivedTransformation();
	const hkTransform& trans3 = testNode3->getDerivedTransformation();
	const hkTransform& trans4 = testNode4->getDerivedTransformation();
*/

	
	
	mSceneGraph->drawAABBs(mDx11Renderer);

	context->IASetInputLayout(vs->getInputLayout());
	context->VSSetShader(vs->getVertexShader(), nullptr, 0u);
	context->PSSetShader(ps->getPixelShader(), nullptr, 0u);
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	bsRenderQueue rq(mDx11Renderer, mResourceManager->getShaderManager());
//	rq.addRenderables(renderables);
	rq.addSceneNode(testNode1);
	rq.addSceneNode(testNode2);
	rq.addSceneNode(testNode3);
	rq.addSceneNode(testNode4);
	rq.draw(mDx11Renderer);

	static auto textManager = mResourceManager->getTextManager();
	static auto snowman = textManager->createText(L"\u2603");
	snowman->setFontSize(192.0f);
//	snowman->setPosition(200.0f, 100.0f);
	snowman->setFlags(FW1_CENTER | FW1_VCENTER | FW1_RESTORESTATE);
	//snowman->addFlags(FW1_ALIASED);
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(mWindow->getHwnd(), &pt);
	snowman->setPosition((float)pt.x, (float)pt.y);
	//snowman->draw();

	static bsRenderStats stats;
	stats.setFps(1.0f/deltaTime * 1000.0f);
	stats.setFrameTime(deltaTime);

	static auto statsText = textManager->createText(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);
	//statsText->setPosition(0.5f, 0.5f);
	statsText->setText(stats.getStatsString());
	//statsText->addFlags(FW1_ALIASED);
	//statsText->draw();

	static std::shared_ptr<bsTextBox> textBox = textManager->createTextBox(4500.0f, 10);
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);

	static int wait = 0;
	static int freq = 50;
	++wait;
	if (wait % freq == 0)
	{
		std::stringstream randomString;
		
		unsigned int charCount = bsMath::randomRange(1, 6);
		for (unsigned int i = 0u; i < charCount; ++i)
		{
			unsigned int stringCount = bsMath::randomRange(1, 7);
			for (unsigned int i = 0u; i < stringCount; ++i)
			{
				randomString << (char)bsMath::randomRange(65, 122);
			}
			randomString << " ";
		}
	
		//bsLog::logMessage(randomString.str().c_str(), pantheios::SEV_DEBUG);

		randomString.str("");
		randomString << (deltaTime / 16.66666666666f);
		bsLog::logMessage(randomString.str().c_str());
	}
	auto tb = *textBox.get();

	

	auto lmbd = [=](const char* text) {textBox->addTextLine(text);};
	//auto nff = boost::bind<void>(&not_functor::foo, &nf, _1);
//	auto func3 = boost::bind<void>(&bsTextBox::addTextLine, &tb, _1);
//	auto func2 = boost::bind<void>(&bsTextBox::addTextLine, &textBox);
	//auto func = boost::bind<void>(&bsTextBox::addTextLine, boost::ref(textBox));
	std::string utf8("hola senor!");
	std::wstring utf16 = bsStringUtils::utf8ToUtf16(utf8);
	
	static bool logCallBackOnce = false;
	if (!logCallBackOnce)
	{
		bsLog::addCallback(lmbd);
		logCallBackOnce = true;
	}

	textManager->drawAllTexts(deltaTime);
	

	context->GSSetShader(nullptr, nullptr, 0u);
	context->HSSetShader(nullptr, nullptr, 0u);

	mDx11Renderer->present();
}

bool Application::keyPressed(const OIS::KeyEvent &arg)
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
	if (arg.key == OIS::KC_G)
	{
		mSceneGraph->displayEmptyAabbs = !mSceneGraph->displayEmptyAabbs;
	}
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = true;
	}

	switch (arg.key)
	{
	case OIS::KC_ADD:
		{
			auto pi = mCamera->getProjectionInfo();
			pi.mFieldOfView += 0.025f;
			mCamera->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mCamera->getProjectionInfo();
			pi.mFieldOfView -= 0.025f;
			mCamera->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_M:
		moveDuck = true;
		break;

	case OIS::KC_R:
		resetDuck = true;
		break;
	}


	return true;
}

bool Application::keyReleased(const OIS::KeyEvent &arg)
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

	switch(arg.key)
	{
	case OIS::KC_M:
		moveDuck = false;
		break;

	case OIS::KC_R:
		resetDuck = false;
		break;
	}

	return true;
}

bool Application::mouseMoved(const OIS::MouseEvent &arg)
{
	if (leftMouseDown)
	{
		mCamera->rotateAboutAxis(bsCamera::AXIS_Y, -((float)arg.state.X.rel * 0.01f));
		mCamera->rotateAboutAxis(bsCamera::AXIS_X, -((float)arg.state.Y.rel * 0.01f));
	}
	
	return true;
}

bool Application::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
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
