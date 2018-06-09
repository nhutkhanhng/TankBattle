class BulletClient : public Bullet
{
public:
	static	GameObjectPtr	StaticCreate()		{ return GameObjectPtr( new BulletClient() ); }

	virtual void		Read( InputMemoryBitStream& inInputStream ) override;
	virtual bool		HandleCollisionWithTank( Tank* inTank ) override;

protected:
	BulletClient();

private:

	SpriteComponentPtr	mSpriteComponent;
};