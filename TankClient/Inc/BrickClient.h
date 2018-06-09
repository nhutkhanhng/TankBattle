class BrickClient : public Brick
{
public:
	static	GameObjectPtr	StaticCreate()		{ return GameObjectPtr( new BrickClient() ); }

protected:
	BrickClient();

private:

	SpriteComponentPtr	mSpriteComponent;
};