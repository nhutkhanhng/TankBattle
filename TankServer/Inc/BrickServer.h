class BrickServer : public Brick
{
public:
	static GameObjectPtr	StaticCreate() { return NetworkManagerServer::sInstance->RegisterAndReturn( new BrickServer() ); }
	void HandleDying() override;
	virtual bool		HandleCollisionWithTank( Tank* inTank ) override;

protected:
	BrickServer();

};