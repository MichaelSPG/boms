#ifndef RENDERABLE_H
#define RENDERABLE_H


class __declspec(novtable) Renderable
{
public:
	virtual ~Renderable() {}

	virtual void setPosition(float x, float y, float z) = 0;

	virtual void translate(float x, float y, float z) = 0;
};

#endif