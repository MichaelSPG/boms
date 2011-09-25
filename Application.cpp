#include "StdAfx.h"

#include "Application.h"

#include <algorithm>
#include <functional>

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>
#include <Physics/Utilities/Dynamics/Keyframe/hkpKeyFrameUtility.h>

#include "bsLog.h"
#include "bsScene.h"
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
#include "bsPrimitiveCreator.h"
#include "bsRayCastUtil.h"
#include "bsSmoothCameraMovement.h"
#include "bsText3D.h"

#include "bsDeferredRenderer.h"

#include "bsAssert.h"
#include "bsScene.h"

#include "bsCharacterController.h"


bsSmoothCameraMovement* camMov;

Application::Application(HINSTANCE hInstance, int showCmd, const int windowWidth,
	const int windowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mCameraSpeed(1.0f)
	, mFreeCamMode(false)
{
	w = a = s = d = space = c = shift = false;
	rightMouseDown = leftMouseDown = mQuit = false;

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


	bsWindow* window = mCore->getWindow();
	
	float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	mCore->getDx11Renderer()->setRenderTargetClearColor(clearColor);

	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	
	mScene = new bsScene(mCore->getDx11Renderer(), mCore->getResourceManager(),
		mCore->getHavokManager(), coreCInfo);

	mDeferredRenderer = new bsDeferredRenderer(mCore->getDx11Renderer(),
		mScene->getCamera(), mCore->getResourceManager()->getShaderManager(),
		window);
	//Register the text manager with the rendering system so that it'll draw texts
	mDeferredRenderer->registerEndOfRenderCallback(std::bind(&bsTextManager::drawAllTexts, textManager));

	mCore->setRenderSystem(mDeferredRenderer);
	mCore->getRenderQueue()->setCamera(mScene->getCamera());

	mPrimCreator = new bsPrimitiveCreator(*mCore->getResourceManager()->getMeshCache());

	bsLog::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);

	

	//////////////////////////////////////////////////////////////////////////


	bsCharacterControllerCInfo cInfo;
	cInfo.world = mScene->getPhysicsWorld();
	cInfo.maxRunSpeed = 50.0f;
	cInfo.mass = 1.0f;
	cInfo.jumpHeight = 2.5f;
	cInfo.airSpeed = 100.0f;
	cInfo.gravityStrength = 3.0f;
	cc = new bsCharacterController(cInfo);


	ccNode = new bsSceneNode();
	bsSceneNode* camNode = mScene->getCamera()->getEntity()->getOwner();
	camNode->setLocalPosition(XMVectorSet(0.0f, cInfo.eyeHeight, 0.0f, 0.0f));
	mScene->addSceneNode(ccNode);
	camNode->setParentSceneNode(ccNode);

	ccNode->setPosition(XMVectorSet(1.0f, 3.5f, -13.0f, 0.0f));
	ccNode->getEntity().attach(cc->getRigidBody());

	//camNode->setLocalRotation(XMVectorSet(-0.004, -0.973f, 0.23f, 0.015f));


	createMeshes();
	createSomeLights();
	createTexts();
	createNodes();
	createWalls(100.0f, 100.0f, 25.0f);
	createKeyframedRb();
	createLines();


	bsSceneNode* box = mPrimCreator->createBox(XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f));
	box->setLocalPosition(XMVectorSet(2.5f, 2.0f, 0.0f, 0.0f));
	box->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);

	bsSceneNode* sphere = mPrimCreator->createSphere(0.5f);
	sphere->setLocalPosition(XMVectorSet(5.0f, 1.0f, 0.0f, 0.0f));
	sphere->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);


	mScene->addSceneNode(box);
	mScene->addSceneNode(sphere);

	//plane->setLocalRotation(hkQuaternion(hkVector4(1.0f, 0.0f, 0.0f), 0.1f));
	/*
	plane->getEntity().mRigidBody->getWorld()->lock();
	plane->getEntity().mRigidBody->setLocalRotation(hkQuaternion::getIdentity());
	plane->getEntity().mRigidBody->getWorld()->unlock();
	*/
	/*
	bsSceneNode* test = new bsSceneNode();
	test->getEntity().attach(mCore->getResourceManager()->getMeshCache()->getMesh("bun_zipper.bsm"));
	test->setLocalScaleUniform(15.0f);
	test->setLocalPosition(hkVector4(10.0f, 10.0f, 10.0f));
	
	mScene->addSceneNode(test);
	*/

	const float camMoveSpeed = 10.0f;
	camMov = new bsSmoothCameraMovement(mScene->getCamera()->getEntity()->getOwner(),
		camMoveSpeed, camMoveSpeed, camMoveSpeed);

	{
		bsSceneNode* lightNode = new bsSceneNode();
		bsPointLightCInfo ci;
		ci.color = XMFLOAT3(1.0f, 0.0f, 1.0f);
		ci.radius = 10.0f;
		ci.intensity = 0.75f;
		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), ci);

		lightNode->getEntity().attach(light);
		mScene->addSceneNode(lightNode);

		nodes["light"] = lightNode;
	}

	{
		D3D11_SAMPLER_DESC samDesc;
		memset(&samDesc, 0, sizeof(samDesc));

		samDesc.MipLODBias = 0.0f;
		samDesc.BorderColor[0] = samDesc.BorderColor[1] = samDesc.BorderColor[2] = samDesc.BorderColor[3] = 0;

		samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samDesc.MaxAnisotropy = 4;
		samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samDesc.MaxLOD = 0.0f;
		samDesc.MinLOD = 0.0f;

		ID3D11SamplerState* sampler = nullptr;
		mCore->getDx11Renderer()->getDevice()->CreateSamplerState(&samDesc, &sampler);

		mCore->getDx11Renderer()->getDeviceContext()->PSSetSamplers(3, 1, &sampler);
	}


	bsSceneNode* node1 = new bsSceneNode();
	mScene->addSceneNode(node1);
	bsSceneNode* node2 = new bsSceneNode();
	mScene->addSceneNode(node2);
	node2->setParentSceneNode(node1);

	node1->getEntity().attach(mMeshes["cube"]);
	node2->getEntity().attach(mMeshes["cube"]);

	node2->setLocalPosition(XMVectorSet(0.0f, 1.01f, 0.0f, 0.0f));
	node1->setPosition(XMVectorSet(0.0f, 0.5f, 0.0f, 0.0f));
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);
	/*
	for (auto itr = nodes.begin(), end = nodes.end(); itr != end; ++itr)
	{
		delete itr->second;
	}
	*/
	delete mScene;
	delete mDeferredRenderer;
	delete mCore;
}

bool goingUp = true;

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	bsCamera* camera = mScene->getCamera();
	bsSceneNode* camNode = camera->getEntity()->getOwner();
	
	
	float right = a ? (-1.0f) : (d ? 1.0f : 0.0f);
	float forward = s ? (-1.0f) : (w ? 1.0f : 0.0f);
	float up = c ? (-1.0f) : (space ? 1.0f : 0.0f);
	
	//camMov->update(forward, right, up, deltaTime);

	mRenderStats.setFps(1.0f / deltaTime * 1000.0f);
	mRenderStats.setFrameTime(deltaTime);

	const XMMATRIX& camTransform = camNode->getTransform();
	const XMVECTOR dir = XMVectorSet(camTransform._13, camTransform._23, camTransform._33, 0.0f);

	if (mFreeCamMode)
	{
		camMov->update(forward, right, up, deltaTime);
	}
	else
	{
		cc->step(16.66667f, forward, -right, space, bsMath::toHK(dir));
	}


	//Update texts
	mCore->getResourceManager()->getTextManager()->updateTexts(deltaTime);

	mTexts["stats"]->setText(mRenderStats.getStatsString());
	mTexts["frameStats"]->setText(mDeferredRenderer->getRenderQueue()->getFrameStats()
		.getFrameStatsStringWide());

	XMFLOAT4A camPos;
	XMStoreFloat4A(&camPos, camNode->getPosition());
	XMFLOAT4A camRot;
	XMStoreFloat4A(&camRot, camNode->getRotation());
	
	wchar_t cameraInfoBuffer[1000] = { 0 };
	swprintf_s(cameraInfoBuffer, 1000, L"Cam pos: x: %.3f, y: %.3f, z: %.3f\n"
		L"Cam rot: x: %.3f, y: %.3f, z: %.3f, w: %.3f", camPos.x, camPos.y, camPos.z,
		camRot.x, camRot.y, camRot.z, camRot.w);
	
	mTexts["camPos"]->setText(cameraInfoBuffer);


	if (leftMouseDown)
	{
		bsSceneNode* lineNode = nodes["line"];
		bsLine3D* line = lineNode->getEntity().getComponent<bsLine3D*>();
		const float rayLength = 100.0f;

		hkpWorldRayCastOutput output;
		XMVECTOR destinationOut, originOut;
		output = camera->screenPointToWorldRay(XMFLOAT2((float)mMouse->getMouseState().X.abs,
			(float)mMouse->getMouseState().Y.abs), rayLength, destinationOut, originOut);

		if (output.hasHit())
		{
			XMVECTOR hitPoint = XMVectorLerp(originOut, destinationOut, output.m_hitFraction);

			XMFLOAT3 points[2];
			//XMStoreFloat3(points, camPos);
			XMStoreFloat3(points + 1, hitPoint);

			XMFLOAT3 normal;
			output.m_normal.store<3, HK_IO_NATIVE_ALIGNED>(&normal.x);
			XMVECTOR normalFromHitPoint = XMLoadFloat3(&normal);
			normalFromHitPoint = XMVectorAdd(normalFromHitPoint, hitPoint);
			XMStoreFloat3(points, normalFromHitPoint);

			{
				XMStoreFloat3(points, originOut);
				
				XMStoreFloat3(points + 1, hitPoint);
				//XMStoreFloat3(points + 1, destinationOut);

				//XMStoreFloat3(points, destinationOut);
				//++points[0].y;
			}
			

			//XMStoreFloat3(points, destination2);
			//XMStoreFloat3(points + 1, origin);

			line->clear();
			line->setColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
			line->addPoints(points, 2);
			{
				XMStoreFloat3(points, hitPoint);
				XMStoreFloat3(points + 1, normalFromHitPoint);
				++points[1].y;
				line->addPoints(points, 2);
			}
			line->build(mCore->getDx11Renderer());

			hkpRigidBody* rb = hkpGetRigidBody(output.m_rootCollidable);
			rb->getWorld()->markForWrite();
			if (rb->getMotionType() != hkpMotion::MOTION_FIXED)
			{
				XMVECTOR impulse = XMVectorSubtract(destinationOut, originOut);
				impulse = XMVector3Normalize(impulse);
				impulse = XMVectorScale(impulse, 10.0f * rb->getMass());
				//rb->applyPointImpulse(bsMath::toHK(impulse), bsMath::toHK(hitPoint));
				rb->applyForce(0.0166666666666f, bsMath::toHK(impulse), bsMath::toHK(hitPoint));
			}
			rb->getWorld()->unmarkForWrite();

			nodes["light"]->setPosition(XMVectorAdd(hitPoint, XMLoadFloat3(&normal)));

			
			bsSceneNode* textNode = nodes["text"];
			textNode->setPosition(XMLoadFloat3(points + 1));
			bsText3D* text = textNode->getEntity().getComponent<bsText3D*>();
			//wchar_t pointText[35];
			//swprintf_s(pointText, L"[%3.3f, %3.3f, %3.3f]", points[1].x, points[1].y, points[1].z);

			wchar_t buf[200];
			swprintf_s(buf, L"mass: %.1f", rb->getMass());
			text->setText(buf);
		}
	}

	//Keyframed RB
	{
		static float accumulatedDt = mCurve->getEnd();
		const float speed = (1.0f / mCurve->getEnd()) * 50000.0f;
		if (!mKeyboard->isKeyDown(OIS::KC_G))
			accumulatedDt -= 0.01666667f * speed;

		while (accumulatedDt < 0.0f)
		{
			accumulatedDt += mCurve->getEnd();
		}
		
		const float invDeltaTimeSecs = 1.0f / 0.016666667f;

		hkpRigidBody* rb = nodes["lift"]->getEntity().mRigidBody;
		rb->getWorld()->markForWrite();
		hkQuaternion nextRotation = rb->getRotation();
		nextRotation.mul(hkQuaternion(hkVector4(0.0f, 1.0f, 0.0f), 0.0025f));
		nextRotation.setIdentity();

		hkVector4 nextPosition = rb->getPosition();
		{
			mCurve->getPoint(accumulatedDt, nextPosition);
		}
		const hkVector4 delta(0.0f, 0.02f, 0.0f);
		if (goingUp)
		{
			nextPosition.add(delta);
			if (nextPosition.getComponent<1>().getReal() > 10.0f)
			{
				goingUp = false;
			}
		}
		else
		{
			nextPosition.sub(delta);
			if (nextPosition.getComponent<1>().getReal() < 0.1f)
			{
				goingUp = true;
			}
		}

		hkpKeyFrameUtility::applyHardKeyFrame(nextPosition, nextRotation, invDeltaTimeSecs,
			rb);
		rb->getWorld()->unmarkForWrite();
	}

	mScene->update(deltaTime);

	if (!mCore->update(deltaTime))
	{
		mQuit = true;
	}
}
#include "bsAlignedAllocator.h"
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
			auto pi = mScene->getCamera()->getProjectionInfo();
			pi.mFieldOfView += 0.25f;
			mScene->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mScene->getCamera()->getProjectionInfo();
			pi.mFieldOfView -= 0.25f;
			mScene->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_V:
		mCore->getDx11Renderer()->setVsyncEnabled(!mCore->getDx11Renderer()->getVsyncEnabled());
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

	case OIS::KC_X:
		createSpheres(25);
		break;

	case OIS::KC_N:
		createBoxes(true, 25);
		break;

	case OIS::KC_B:
		createBoxes(false, 25);
		break;

	case OIS::KC_R:
		{
			hkPseudoRandomGenerator rng(GetTickCount());
			hkVector4 p;
			rng.getRandomVector11(p);
			hkQuaternion q;
			rng.getRandomRotation(q);


			nodes["line"]->setPosition(bsMath::toXM(p));
			nodes["line"]->setLocalRotation(bsMath::toXM(q));
		}
		break;

	case OIS::KC_F:
		toggleFreeCam();
		break;

	case OIS::KC_P:
		mScene->setStepPhysics(!mScene->getStepPhysics());
		break;
	}

	return true;
}

void Application::toggleFreeCam()
{
	bsSceneNode* camNode = mScene->getCamera()->getEntity()->getOwner();
	//const XMVECTOR camPosition = camNode->getPosition();
	const XMVECTOR camRotation = camNode->getRotation();

	if (!camNode->getParentSceneNode())
	{
		//In free cam mode.
		camNode->setPosition(XMVectorZero());
		camNode->setLocalPosition(XMVectorSet(0.0f, 1.1f, 0.0f, 0.0f));
		//camNode->setLocalRotation(XMQuaternionIdentity());

		camNode->setParentSceneNode(ccNode);

		mFreeCamMode = false;
	}
	else
	{
		//Attached to controller.
		camNode->setParentSceneNode(nullptr);
		//camNode->setPosition(camPosition);
		camNode->setLocalRotation(camRotation);

		mFreeCamMode = true;
	}
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

XMFLOAT3 yawPitchRoll = XMFLOAT3(0.0f, 0.0f, 0.0f);

bool Application::mouseMoved(const OIS::MouseEvent& arg)
{
	bsCamera* camera = mScene->getCamera();
	bsSceneNode* camNode = camera->getEntity()->getOwner();
	const XMVECTOR& cameraRot = camNode->getRotation();

	if (rightMouseDown)
	{
		float rotationX = (float)arg.state.X.rel;
		float rotationY = (float)arg.state.Y.rel;

		const float sensitivity = 0.005f;

		//camera->rotateX(-rotationX * sensitivity);
		//camera->rotateY(-rotationY * sensitivity);

		//const XMVECTOR cameraXAxis = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), cameraRot);
		//const XMVECTOR cameraYAxis = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), cameraRot);

		//const XMVECTOR rotX = XMQuaternionRotationAxis(cameraXAxis, XMConvertToRadians(rotationX * sensitivity));
		//const XMVECTOR rotY = XMQuaternionRotationAxis(cameraYAxis, XMConvertToRadians(rotationY * sensitivity));

		const XMVECTOR rotX = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(rotationY * sensitivity));
		const XMVECTOR rotY = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(rotationX * sensitivity));

		XMVECTOR finalRot = XMQuaternionMultiply(rotX, rotY);
		finalRot = XMQuaternionMultiply(finalRot, cameraRot);

		finalRot = XMQuaternionMultiply(cameraRot, XMQuaternionRotationNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(rotationX * sensitivity)));
		
		yawPitchRoll.x += rotationX * sensitivity;
		yawPitchRoll.y += rotationY * sensitivity;

		finalRot = XMQuaternionRotationRollPitchYaw(yawPitchRoll.y, yawPitchRoll.x, 0.0f);
		
		camNode->setLocalRotation(XMQuaternionInverse(finalRot));
	}


	float rotationZ = bsMath::clamp(-5.0f, 5.0f, (float)arg.state.Z.rel);
	if (mFreeCamMode)
	{
		XMVECTOR scroll = XMVectorSet(0.0f, 0.0f, rotationZ, 0.0f);
		scroll = XMVector3InverseRotate(scroll, cameraRot);
		scroll = XMVectorAdd(scroll, camNode->getPosition());
		camNode->setPosition(scroll);
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

	case OIS::MB_Middle:
		nodes["line"]->getEntity().getComponent<bsLine3D*>()->clear();
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

void Application::createSomeLights()
{
	std::vector<bsSceneNode*> nodes_(5);
	XMFLOAT3 lightColors[5] = 
	{
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < nodes_.size(); ++i)
	{
		bsPointLightCInfo ci;
		ci.color = lightColors[i];
		ci.intensity = 1.0f;
		ci.radius = 25.0f;
		bsLight* light = new bsLight(bsLight::LT_POINT,
			mCore->getResourceManager()->getMeshCache(), ci);

		nodes_[i] = new bsSceneNode();
		nodes_[i]->getEntity().attach(light);
	}
	//auto torus = mCore->getResourceManager()->getMeshCache()->getMesh("torus_knot.bsm");
	//nodes_[0]->attachRenderable(torus);

	nodes_[0]->setPosition(XMVectorSet(3.0f, 50.0f, 3.0f, 0.0f));
	/*
	nodes_[0]->getEntity().detach<bsLight*>();
	bsPointLightCInfo ci;
	ci.color = lightColors[0];
	ci.intensity = 1.0f;
	ci.radius = 15.0f;
	nodes_[0]->getEntity().attach(new bsLight(bsLight::LT_POINT,
	mCore->getResourceManager()->getMeshCache(), ci));
	*/
	nodes_[0]->setPosition(XMVectorSet(  0.0f, 50.5f,  0.0f, 0.0f));
	nodes_[1]->setPosition(XMVectorSet(  0.0f, 7.50f,  25.0f, 0.0f));
	nodes_[2]->setPosition(XMVectorSet(  0.0f, 7.50f, -25.0f, 0.0f));
	nodes_[3]->setPosition(XMVectorSet( 25.0f, 7.50f,   0.0f, 0.0f));
	nodes_[4]->setPosition(XMVectorSet(-25.0f, 7.50f,   0.0f, 0.0f));

	for (size_t i = 0; i < nodes_.size(); ++i)
	{
		mScene->addSceneNode(nodes_[i]);
	}

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
}

void Application::createMeshes()
{
	bsMeshCache* meshCache = mCore->getResourceManager()->getMeshCache();

	mMeshes.insert(std::make_pair("roi", meshCache->getMesh("roi.bsm")));
	mMeshes.insert(std::make_pair("teapot", meshCache->getMesh("teapot.bsm")));
	mMeshes.insert(std::make_pair("gourd", meshCache->getMesh("gourd.bsm")));
	mMeshes.insert(std::make_pair("sphere", meshCache->getMesh("sphere_1m_d.bsm")));
	mMeshes.insert(std::make_pair("greeble", meshCache->getMesh("greeble_town_small.bsm")));
	mMeshes.insert(std::make_pair("plane", meshCache->getMesh("plane_1m.bsm")));
	mMeshes["cube"] = meshCache->getMesh("unit_cube.bsm");
}

void Application::createTexts()
{
	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	auto statsText = textManager->createText2D(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);

	
	//Scrolling text
	std::shared_ptr<bsScrollingText2D> textBox(textManager->createScrollingText2D(4500.0f, 10));
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);
	bsLog::addCallback([textBox](const char* message)
	{
		textBox->addTextLine(message);
	});

	auto frameStats = textManager->createText2D(L"");
	frameStats->setPosition(2.5f, 450.0f);
	frameStats->addFlags(FW1_RESTORESTATE);
	frameStats->setColor(0xff00ff00);
	frameStats->setFontSize(14.0f);

	auto camPos = textManager->createText2D(L"");
	camPos->setPosition(850.0f, 2.5f);
	camPos->addFlags(FW1_RESTORESTATE);
	camPos->setColor(0xffffffff);
	camPos->setFontSize(14.0f);

	auto crosshair = textManager->createText2D(L".");
	crosshair->setPosition((float)mCore->getWindow()->getWindowWidth() * 0.5f,
		((float)mCore->getWindow()->getWindowHeight() * 0.5f) - 10.0f);
	crosshair->addFlags(FW1_RESTORESTATE | FW1_CENTER);
	crosshair->setColor(0xffcccccc);
	crosshair->setFontSize(12.0f);


	mTexts.insert(std::make_pair("stats", statsText));
	//mTexts.insert(std::make_pair("scrolling", textBox));
	mTexts.insert(std::make_pair("frameStats", frameStats));
	mTexts.insert(std::make_pair("camPos", camPos));
	mTexts.insert(std::make_pair("crosshair", crosshair));
}
#include <Physics/Collide/Shape/HeightField/Plane/hkpPlaneShape.h>
void Application::createNodes()
{
	const XMVECTOR spherePosition = XMVectorSet(2.5f, 5.0f, 0.0f, 0.0f);

	bsSceneNode* sphereNode = new bsSceneNode();
	sphereNode->setPosition(spherePosition);
	bsEntity& entity = sphereNode->getEntity();
	entity.attach(mMeshes["sphere"]);

	hkpRigidBodyCinfo ci;
	ci.m_shape = new hkpSphereShape(0.5f);
	ci.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;
	ci.m_position = bsMath::toHK(spherePosition);
	ci.m_friction = 0.0f;
	ci.m_restitution = 1.01f;
	ci.m_linearDamping;
	ci.m_mass = (4.0f / 3.0f) * XM_PI * (0.5f * 0.5f * 0.5f);
	hkpRigidBody* sphereRb = new hkpRigidBody(ci);
	ci.m_shape->removeReference();
	entity.attach(sphereRb);

	bsPointLightCInfo plci;
	plci.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	plci.intensity = 1.0f;
	plci.radius = 10.0f;
	bsLight* l = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), plci);
	entity.attach(l);
	
	mScene->addSceneNode(sphereNode);
	nodes.insert(std::make_pair("sphere", sphereNode));
	
	//Light ball
#if 0
	{
		bsSceneNode* lightNode = new bsSceneNode();
		lightNode->setPosition(XMVectorAdd(spherePosition, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
		bsEntity& lightEntity = lightNode->getEntity();
		lightEntity.attach(l);
	
		sphereNode->addChildSceneNode(lightNode);
	}
#endif

#if 0
	bsSceneNode* ground = new bsSceneNode(hkVector4(0.0f, 0.0f, 0.0f));
	bsEntity& groundEntity = ground->getEntity();
	//auto groundMesh = mCore->getResourceManager()->getMeshCache()->getMeshCreator()
//		.createBox(XMFLOAT3(100.0f, 0.1f, 100.0f));
	auto groundMesh = mCore->getResourceManager()->getMeshCache()->getMesh("plane_1m.bsm");
	//groundEntity.attach(groundMesh);

	hkpRigidBodyCinfo ci2;
	ci2.m_shape = new hkpPlaneShape(hkVector4(0.0f, 1.0f, 0.0f), hkVector4(0.0f, 0.0f, 0.0f), hkVector4(50.0f, 0.05f, 50.0f));
	//ci2.m_shape = new hkpBoxShape(hkVector4(50.0f, 0.05f, 50.0f));
	ci2.m_motionType = hkpMotion::MOTION_FIXED;
	ci2.m_position = ground->getPosition();
	hkpRigidBody* groundRb = new hkpRigidBody(ci2);
	groundEntity.attach(groundRb);

	ground->setScale2(XMVectorSet(100.0f, 1.0f, 100.0f, 0.0f));

	mScene->addSceneNode(ground);
	nodes.insert(std::make_pair("ground", ground));
#endif

	bsSceneNode* ground = mPrimCreator->createPlane(XMVectorSet(50.0f, 1.0f, 50.0f, 0.0f));
	mScene->addSceneNode(ground);
	nodes["ground"] = ground;


	bsSceneNode* greebleNode = new bsSceneNode();
	bsEntity& greebleEntity = greebleNode->getEntity();
	//greebleEntity.attach(mMeshes["greeble"]);
	greebleEntity.attach(mCore->getResourceManager()->getMeshCache()->getMesh("greeble_town.bsm"));
	mScene->addSceneNode(greebleNode);
	nodes.insert(std::make_pair("greeble", greebleNode));

	hkPseudoRandomGenerator rng(GetTickCount());
	const float offsetPos = 200.0f;
	for (unsigned int i = 0; i < 100; ++i)
	{
		bsSceneNode* lightNode = new bsSceneNode();
		lightNode->setPosition(XMVectorSet(rng.getRandRange(-offsetPos, offsetPos), 5.0f, rng.getRandRange(-offsetPos, offsetPos), 1.0f));

		bsPointLightCInfo plci;
		plci.color = XMFLOAT3(rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f));
		plci.radius = rng.getRandRange(2.0f, 25.0f);
		plci.intensity = 1.0f;

		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(),
			plci);
		lightNode->getEntity().attach(light);
		mScene->addSceneNode(lightNode);
	}


	bsSceneNode* lineNode = new bsSceneNode();
	bsEntity& lineEntity = lineNode->getEntity();
	bsLine3D* line3D = new bsLine3D(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	lineEntity.attach(line3D);
	nodes.insert(std::make_pair("line", lineNode));
	mScene->addSceneNode(lineNode);

	bsSceneNode* textNode = new bsSceneNode();
	bsEntity& textEntity = textNode->getEntity();
	bsText3D* text3D = new bsText3D(mCore->getDx11Renderer()->getDeviceContext(),
		mCore->getDx11Renderer()->getDevice(), mCore->getResourceManager()->getTextManager()->getFw1Factory());
	textEntity.attach(text3D);
	nodes["text"] = textNode;
	mScene->addSceneNode(textNode);
}

void Application::createSpheres(unsigned int count)
{
	hkPseudoRandomGenerator rng(GetTickCount());
	const hkVector4 minPos(-25.0f,  2.5f, -25.0f);
	const hkVector4 maxPos( 25.0f, 15.0f,  25.0f);

	std::vector<bsSceneNode*> spheres;
	spheres.reserve(count);

	hkVector4 pos;
	float radius;

	bsPointLightCInfo ci;

	for (size_t i = 0; i < count; ++i)
	{
		rng.getRandomVectorRange(minPos, maxPos, pos);
		radius = rng.getRandRange(0.2f, 2.0f);
		const float volume = (4.0f / 3.0f) * XM_PI * (radius * radius * radius);

#if 1
		bsSceneNode* node = new bsSceneNode();
		bsEntity& entity = node->getEntity();
		entity.attach(mMeshes["sphere"]);

		hkpMassProperties massProps;
		hkInertiaTensorComputer::computeSphereVolumeMassProperties(radius, volume, massProps);

		hkpRigidBodyCinfo rbci;
		rbci.setMassProperties(massProps);
		rbci.m_shape = new hkpSphereShape(radius);
		rbci.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;
		rbci.m_position = pos;
		//rbci.m_mass = volume;
		hkpRigidBody* rb = new hkpRigidBody(rbci);
		entity.attach(rb);

		mScene->addSceneNode(node);

		node->setLocalScaleUniform(radius * 2.0f);
#endif
		//bsSceneNode* sphere = mPrimCreator->createSphere(radius);
		node->setLocalPosition(bsMath::toXM(pos));

		/*
		//Detach mesh+attach light

		sphere->getEntity().detach<std::shared_ptr<bsMesh>>();

		ci.color = XMFLOAT3(rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f));
		ci.radius = rng.getRandRange(1.0f, 15.0f);
		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), ci);
		sphere->getEntity().attach(light);
		*/

		//mScene->addSceneNode(sphere);
	}
}

void Application::createBoxes(bool staticBoxes, unsigned int count)
{
	hkPseudoRandomGenerator rng(GetTickCount());
	const hkVector4 minPos(-25.0f,  2.5f, -25.0f);
	const hkVector4 maxPos( 25.0f, 15.0f,  25.0f);

	auto mesh = mCore->getResourceManager()->getMeshCache()->getMesh("unit_cube.bsm");

	hkVector4 pos;
	XMFLOAT4A scale;

	const hkpMotion::MotionType motion = staticBoxes ? hkpMotion::MOTION_FIXED
		: hkpMotion::MOTION_BOX_INERTIA;

	for (size_t i = 0; i < count; ++i)
	{
		rng.getRandomVectorRange(minPos, maxPos, pos);
		scale.x = rng.getRandRange(0.5f, 5.0f);
		scale.y = rng.getRandRange(0.5f, 5.0f);
		scale.z = rng.getRandRange(0.5f, 5.0f);

		const float volume = scale.x * scale.y * scale.z;

#if 1
		bsSceneNode* node = new bsSceneNode();
		bsEntity& entity = node->getEntity();
		entity.attach(mesh);

		hkVector4 halfExtents(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
		const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
			hkConvexShapeDefaultRadius);
		halfExtents.sub(convexRadius);

		hkpMassProperties massProps;
		hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume, massProps);

		hkpRigidBodyCinfo rbci;
		rbci.setMassProperties(massProps);
		rbci.m_shape = new hkpBoxShape(halfExtents);
		rbci.m_motionType = motion;
		rbci.m_position = pos;

		hkpRigidBody* rb = new hkpRigidBody(rbci);
		entity.attach(rb);

		mScene->addSceneNode(node);

		node->setLocalScale(XMLoadFloat4A(&scale));
		node->setPosition(bsMath::toXM(pos));
#endif
		/*
		bsSceneNode* box = mPrimCreator->createBox(XMVectorScale(XMLoadFloat4A(&scale), 0.5f));
		box->setLocalPosition(bsMath::toXM(pos));
		box->getEntity().mRigidBody->setMotionType(motion);
		mScene->addSceneNode(box);
		*/
	}
}

void Application::createWalls(float offsetFromCenter, float length, float height)
{
	const XMVECTOR northPos = XMVectorSet(0.0f, height * 0.5f, offsetFromCenter * 0.5f, 0.0f);
	const XMVECTOR eastPos = XMVectorSet(offsetFromCenter * 0.5f, height * 0.5f, 0.0f, 0.0f);
	const XMVECTOR southPos = XMVectorSet(0.0f, height * 0.5f, -offsetFromCenter * 0.5f, 0.0f);
	const XMVECTOR westPos = XMVectorSet(-offsetFromCenter * 0.5f, height * 0.5f, 0.0f, 0.0f);

	
	bsSceneNode* northNode = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsSceneNode* eastNode = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsSceneNode* southNode = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsSceneNode* westNode = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	northNode->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	eastNode->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	southNode->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	westNode->getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);

	northNode->setLocalPosition(northPos);
	eastNode->setLocalPosition(eastPos);
	southNode->setLocalPosition(southPos);
	westNode->setLocalPosition(westPos);



	XMVECTOR scale = XMVectorSet(length, height, 5.0f, 1.0f);
	northNode->setLocalScale(scale);
	eastNode->setLocalScale(scale);
	southNode->setLocalScale(scale);
	westNode->setLocalScale(scale);

	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(90.0f));
	eastNode->setLocalRotation(rotation);

	rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(180.0f));
	southNode->setLocalRotation(rotation);

	rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(270.0f));
	westNode->setLocalRotation(rotation);


	mScene->addSceneNode(northNode);
	mScene->addSceneNode(eastNode);
	mScene->addSceneNode(southNode);
	mScene->addSceneNode(westNode);
}

void Application::createKeyframedRb()
{
	const XMVECTOR boxSize = XMVectorSet(5.0f, 0.1f, 5.0f, 0.0f);
	const XMVECTOR position = XMVectorSet(10.0f, 0.1f, 0.0f, 0.0f);

	bsSceneNode* keyframedNode = new bsSceneNode();
	keyframedNode->setPosition(position);

	bsEntity& entity = keyframedNode->getEntity();
	entity.attach(mMeshes["cube"]);
	keyframedNode->setLocalScale(boxSize);

	float volume = XMVectorGetX(boxSize) * XMVectorGetY(boxSize) * XMVectorGetZ(boxSize);
	hkVector4 halfExtents(bsMath::toHK(boxSize));
	halfExtents.mul(hkSimdReal::convert(0.5f));

	hkpMassProperties massProps;
	hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume, massProps);

	hkpRigidBodyCinfo ci;
	ci.setMassProperties(massProps);
	ci.m_shape = new hkpBoxShape(bsMath::toHK(XMVectorScale(boxSize, 0.5f)));
	ci.m_motionType = hkpMotion::MOTION_KEYFRAMED;
	ci.m_position = bsMath::toHK(position);

	hkpRigidBody* rb = new hkpRigidBody(ci);
	entity.attach(rb);

	mScene->addSceneNode(keyframedNode);

	nodes["lift"] = keyframedNode;

	float j = 1.0f;
	for (size_t i = 0; i < 10; ++i, j += 1.0f)
	{
		//const float x = (i % 2 == 0) ? 0.1f : -0.1f;
		//const float z = (i % 4 == 0) ? 0.1f : -0.1f;

		halfExtents.set(0.5f, 0.5f, 0.5f);
		const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
			hkConvexShapeDefaultRadius);
		halfExtents.sub(convexRadius);

		volume = (halfExtents.getComponent<0>().getReal() * 2.0f) *
			(halfExtents.getComponent<1>().getReal() * 2.0f) * (halfExtents.getComponent<2>().getReal() * 2.0f);

		hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume * 100.0f, massProps);

		hkpRigidBodyCinfo ci;
		ci.setMassProperties(massProps);
		ci.m_shape = new hkpBoxShape(halfExtents);
		ci.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
		//ci.m_mass = 10.0f;
		hkpRigidBody* rb = new hkpRigidBody(ci);

		bsSceneNode* box = new bsSceneNode();
		box->getEntity().attach(rb);
		box->getEntity().attach(mMeshes["cube"]);

		//const XMVECTOR boxPosition = XMVectorAdd(position, XMVectorSet(x, (j + 1.1001f) * 1.01f, z, 0.0f));
		const XMVECTOR boxPosition = XMVectorAdd(position, XMVectorSet(0.0f, j * 1.0001f, 0.0f, 0.0f));
		box->setPosition(boxPosition);
		//box->setLocalScale(XMVectorSet(2.1f, 2.1f, 2.1f, 0.0f));
		mScene->addSceneNode(box);
	}
}

void Application::createLines()
{
	mCurve = new hkpLinearParametricCurve();
	mCurve->setSmoothingFactor(0.5f);

	const unsigned int pointCount = 1500;
	const unsigned int extraPointCount = 100;
	const float pointIncr = 1.0f / pointCount;

	const float twists = 3.0f;
	const float height = 15.0f;
	const float radius = 10.0f;
	hkVector4 point;

	float f = 1.0f;
	for (unsigned int t = 0; t < pointCount; ++t, f -= pointIncr)
	{
		const float angle = f * XM_PI * 2.0f * twists;
		point.set(cosf(angle) * radius * (1.0f + f), f * height, sinf(angle) * radius * (1.0f + f));

		mCurve->addPoint(point);
	}

	{
		hkVector4 startPoint, endPoint, currentPosition;
		mCurve->getPoint(mCurve->getStart(), startPoint);
		mCurve->getPoint(mCurve->getEnd(), endPoint);

		const float extraPointIncr = 1.0f / extraPointCount;
		f = 0.0f;
		for (unsigned int i = 0; i < extraPointCount; ++i, f += extraPointIncr)
		{
			currentPosition.setInterpolate(endPoint, startPoint, hkSimdReal::convert(f));
			mCurve->addPoint(currentPosition);
		}
	}
	
	//Make closed.
		f = 1.0f;
	const float angle = f * XM_PI * 2.0f * twists;
	point.set(cosf(angle) * radius * (1.0f + f), f * height, sinf(angle) * radius * (1.0f + f));
	mCurve->addPoint(point);

	mCurve->setClosedLoop(true);


	std::vector<XMFLOAT3> pointList;
	pointList.reserve((pointCount * 2) + 2 + extraPointCount);

	XMFLOAT3 p;
	hkVector4 pointOnCurve;
	f = 0.0f;
	for (unsigned int i = 0; i < pointCount + extraPointCount; ++i, f += 1.0f)
	{
		mCurve->getPoint(f, pointOnCurve);
		pointOnCurve.store<3, HK_IO_NATIVE_ALIGNED>(&p.x);
		pointList.push_back(p);

		mCurve->getPoint(f + 1.0f, pointOnCurve);
		pointOnCurve.store<3, HK_IO_NATIVE_ALIGNED>(&p.x);
		pointList.push_back(p);
	}

	bsSceneNode* node = new bsSceneNode();
	bsLine3D* line = new bsLine3D(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
	line->addPoints(&pointList[0], pointList.size());
	line->build(mCore->getDx11Renderer());
	node->getEntity().attach(line);
	mScene->addSceneNode(node);
	nodes["mCurve"] = node;
}
