#ifndef ENTITY_H
#define ENTITY_H


class Entity
{
public:
	Entity();
	virtual ~Entity() = 0;

	virtual void update(float deltaTime) = 0;

	virtual void render() = 0;

	virtual bool isActive()
	{
		return mActive;
	}

protected:
	unsigned int mID;
	bool mActive;
};

#endif // ENTITY_H