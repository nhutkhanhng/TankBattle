class Bullet : public GameObject
{
public:

	CLASS_IDENTIFICATION( 'BULT', GameObject )

	enum EBulletReplicationState
	{
		EBRS_Pose			= 1 << 0,
		EBRS_Color			= 1 << 1,
		EBRS_PlayerId		= 1 << 2,

		EBRS_AllState	= EBRS_Pose | EBRS_Color | EBRS_PlayerId
	};

	static	GameObject*	StaticCreate() { return new Bullet(); }
	virtual uint32_t	GetAllStateMask()	const override	{ return EBRS_AllState; }
	virtual uint32_t	Write( OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState ) const override;

	void			SetVelocity( const Vector3& inVelocity )	{ mVelocity = inVelocity; }
	const Vector3&	GetVelocity() const					{ return mVelocity; }
	void		SetPlayerId( int inPlayerId )	{ mPlayerId = inPlayerId; }
	int			GetPlayerId() const				{ return mPlayerId; }

	void		InitFromShooter( Tank* inShooter );

	virtual void Update() override;

	virtual bool HandleCollisionWithTank( Tank* inCat ) override;

protected:
	Bullet();

	Vector3		mVelocity;
	float		mMuzzleSpeed;
	int			mPlayerId;

};

typedef shared_ptr<Bullet>	BulletPtr;