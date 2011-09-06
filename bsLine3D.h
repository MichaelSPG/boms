#pragma once


#include <vector>

#include <Common/Base/hkBase.h>

#include "bsRenderable.h"
#include "bsMath.h"

struct ID3D11Buffer;
struct ID3D11Buffer;
class bsDx11Renderer;


/*	This class can be used for drawing line lists, meaning each line consists of 2 unique
	points that will not be used for the next and previous lines.
*/
class bsLine3D : public bsRenderable
{
	friend class bsRenderQueue;

public:
	bsLine3D(const XMFLOAT4& colorRgba);

	~bsLine3D();

	inline RenderableType getRenderableType() const
	{
		return LINE;
	}

	//Returns true if there are are least 2 points and an even number of points.
	//TODO: Rename this function.
	inline bool hasFinishedLoading() const
	{
		return mPoints.size() && ((mPoints.size() & 1) == 0);
	}


	/*	Add a single point to the collection.
		Use the addPoints function when adding many points at once for greater efficiency.
	*/
	void addPoint(const XMFLOAT3& position);

	/*	Adds a vector of points to the collection.
	*/
	void addPoints(const std::vector<XMFLOAT3>& points);

	/*	Creates the buffers used for drawing. Call this before trying to render this object.
		Returns true on success.
		There must be at least 2 points, and an even number of points.
	*/
	bool create(bsDx11Renderer* dx11Renderer);

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

private:
	void draw(bsDx11Renderer* dx11Renderer);

	bool		mFinished;
	XMFLOAT4	mColor;

	std::vector<XMFLOAT3>	mPoints;

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;
};
