#pragma once

#include <vector>

#include <d3d11.h>
#include <xnamath.h>

#include "bsCollision.h"

struct ID3D11Buffer;
struct ID3D11Buffer;
class bsDx11Renderer;


/*	This class can be used for drawing line lists, meaning each line consists of 2 unique
	points that will not be used for the next and previous lines.

	Lines ignore scale.
*/
class bsLineRenderer
{
	friend class bsRenderQueue;

public:
	bsLineRenderer(const XMFLOAT4& colorRgba);

	~bsLineRenderer();

	/*	Returns true if there are are least 2 points in the list of points, there are an
		even number of points, and the lines has been built.
	*/
	inline bool hasBeenBuilt() const
	{
		return mPoints.size() && ((mPoints.size() & 1) == 0) && mFinished;
	}


	/*	Add a single point to the collection.
		Use the addPoints function when adding many points at once for greater efficiency.
	*/
	void addPoint(const XMFLOAT3& position);

	/*	Adds an array of points to the collection.
	*/
	void addPoints(const XMFLOAT3* points, unsigned int pointCount);

	/*	Creates the buffers used for drawing. Call this before trying to render this object.
		Returns true on success.
		There must be at least 2 points, and an even number of points.
	*/
	bool build(bsDx11Renderer* dx11Renderer);

	//Returns number of defined points.
	inline unsigned int getPointCount() const
	{
		return mPoints.size();
	}

	inline const XMFLOAT4& getColor() const
	{
		return mColor;
	}

	inline void setColor(const XMFLOAT4& color)
	{
		mColor = color;
	}

	/*	Clears all previously entered points.
	*/
	inline void clear()
	{
		mPoints.clear();

		if (mVertexBuffer)
		{
			mVertexBuffer->Release();
			mVertexBuffer = nullptr;
		}
		if (mIndexBuffer)
		{
			mIndexBuffer->Release();
			mIndexBuffer = nullptr;
		}
	}

private:
	void draw(bsDx11Renderer* dx11Renderer);

	bool		mFinished;
	XMFLOAT4	mColor;

	std::vector<XMFLOAT3>	mPoints;

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
};
