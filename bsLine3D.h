#ifndef BS_LINE3D_H
#define BS_LINE3D_H

#include "bsConfig.h"

#include <vector>

#include "bsRenderable.h"
#include "bsMath.h"

struct ID3D11Buffer;
struct ID3D11Buffer;
class bsDx11Renderer;

/*	This class can be used for drawing line lists, ie each line consists of 2 unique
	points that will not be used for the next and previous lines.
	
*/
class bsLine3D : public bsRenderable
{
	friend class bsRenderQueue;

public:
	bsLine3D(const XMFLOAT4& color);

	~bsLine3D();

	inline const RenderableIdentifier getRenderableIdentifier() const
	{
		return LINES;
	}

	void addPoint(const XMFLOAT3& position);

	void addPoints(const std::vector<XMFLOAT3>& points);

	//Creates the buffers. Call this before trying to render this object.
	//Returns true on success. It is assumed that isOk returns true, meaning that there
	//must be at least 2 points and an even number of points.
	bool create(bsDx11Renderer* dx11Renderer);

	//Returns true if there are are least 2 points and an even number of points.
	bool isOkForRendering() const
	{
		return mPoints.size() && ((mPoints.size() & 1) == 0);
	}

	inline unsigned int getPointCount() const
	{
		return mPoints.size();
	}

	//Read/write access
	inline XMFLOAT4& getColor()
	{
		return mColor;
	}

	//Read only
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

#endif // BS_LINE3D_H
