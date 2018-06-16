class Brick : public GameObject
{
public:
	CLASS_IDENTIFICATION( 'BRIK', GameObject )

	enum EBrickReplicationState
	{
		EBrRS_Pose		= 1 << 0,
		EBrRS_Color		= 1 << 1,

		EBrRS_AllState	= EBrRS_Pose | EBrRS_Color
	};

	static	GameObject*	StaticCreate() { return new Brick(); }

	virtual uint32_t	GetAllStateMask()	const override	{ return EBrRS_AllState; }

	virtual uint32_t	Write( OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState ) const override;
	virtual void		Read( InputMemoryBitStream& inInputStream ) override;

	virtual bool HandleCollisionWithTank( Tank* Tank ) override;

protected:
	Brick();

};