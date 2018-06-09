class TankClient : public Tank
{
public:
	static	GameObjectPtr	StaticCreate()		{ return GameObjectPtr( new TankClient() ); }

	virtual void Update();
	virtual void	HandleDying() override;

	virtual void	Read( InputMemoryBitStream& inInputStream ) override;

	void DoClientSidePredictionAfterReplicationForLocalTank( uint32_t inReadState );
	void DoClientSidePredictionAfterReplicationForRemoteTank( uint32_t inReadState );


protected:
	TankClient();


private:

	void InterpolateClientSidePrediction( float inOldRotation, const Vector3& inOldLocation, const Vector3& inOldVelocity, bool inIsForRemoteCat );

	float				mTimeLocationBecameOutOfSync;
	float				mTimeVelocityBecameOutOfSync;
	
	SpriteComponentPtr	mSpriteComponent;
};