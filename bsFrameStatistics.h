#pragma once


struct bsFrameStatistics
{
	struct RenderingInfo
	{
		float cullDuration;
		float opaqueDuration;
		float transparentDuration;
		float lightDuration;
		float lineDuration;
		float accumulateDuration;
		float fxaaDuration;
		float textDuration;

		float presentDuration;
		float additionalStateChangeOverheadDuration;

		float totalRenderingDuration;
	};

	struct PhysicsInfo
	{
		float stepDuration;
		float synchronizationDuration;
		unsigned int numActiveRigidBodies;
		unsigned int numActiveSimulationIslands;
	};

	RenderingInfo renderingInfo;
	PhysicsInfo physicsInfo;
};
