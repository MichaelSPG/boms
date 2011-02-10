#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "VertexTypes.h"
#include "Renderable.h"

class Triangle : public Renderable
{
public:
	Triangle(SVertex verticesArray[3])
	{
		for (int i = 0; i < 3; i++)
		{
			mVertices[i].x = verticesArray[i].x;
			mVertices[i].y = verticesArray[i].y;
			mVertices[i].z = verticesArray[i].z;
			mVertices[i].w = verticesArray[i].w;
			mVertices[i].color = verticesArray[i].color;
		}
	}

	~Triangle() {}

	SVertex* getVertices()
	{
		return mVertices;
	}

	virtual void setPosition(float x, float y, float z)
	{

	}

	virtual void translate(float x, float y, float z)
	{
		for (int i = 0; i < 3; i++)
		{
			mVertices[i].x += x;
			mVertices[i].y += y;
			mVertices[i].z += z;
		}
	}

private:
	SVertex mVertices[3];
};

#endif