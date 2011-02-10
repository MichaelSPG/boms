#ifndef MATRIX_STACK_H
#define MATRIX_STACK_H

//#include <xnamath.h>
#include "Dx11Renderer.h"


class MatrixStack
{
public:
	inline void push(const XMMATRIX* matrix)
	{
		//mMatrices.push_back(matrix);
	}

	inline void pop()
	{
		mMatrices.pop_back();
	}

	inline const std::vector<XMMATRIX*>& getStack() const
	{
		return mMatrices;
	}

private:
	std::vector<XMMATRIX*> mMatrices;
};

#endif // MATRIX_STACK_H