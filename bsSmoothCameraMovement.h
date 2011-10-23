#pragma once

class bsTransform;


/*	Smoothly moves a camera by using acceleration for moving it, resulting in a smoother
	transition from stationary to moving than used fixed translation values would.
*/
class bsSmoothCameraMovement
{
public:
	bsSmoothCameraMovement(bsTransform& cameraTransform, float maxForwardSpeed,
		float maxRightSpeed, float maxUpSpeed);

	~bsSmoothCameraMovement();

	/*	Speeds are typically -1 or 1 when a button is pressed, and 0 if the button is not
		pressed.
		This should be called every frame, even when not moving, since the algorithm used
		to accelerate needs to know about frames where no motion occurs.
	*/
	void update(float forwardSpeed, float rightSpeed, float upSpeed, float deltaTimeMs);

private:
	bsTransform*	mCameraTransform;

	float mCurrentForwardSpeed;
	float mCurrentRightSpeed;
	float mCurrentUpSpeed;

	float mMaxForwardSpeed;
	float mMaxRightSpeed;
	float mMaxUpSpeed;
};
