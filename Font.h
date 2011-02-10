#ifndef FONT_H
#define FONT_H




class Font : public Renderable
{
public:
	Font() :
		mFont(nullptr)
	{
		
	}

	virtual void setPosition(float x, float y, float z) {}

	virtual void translate(float x, float y, float z) {}
	
	ID3DXFont* getFont() const
	{
		return mFont;
	}

private:
	ID3DXFont	*mFont;
};

#endif