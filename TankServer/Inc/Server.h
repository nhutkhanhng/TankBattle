class Server : public Engine
{
public:

	static bool StaticInit();

	virtual void DoFrame() override;

	virtual int Run();

	void HandleNewClient( ClientProxyPtr inClientProxy );
	void HandleLostClient( ClientProxyPtr inClientProxy );

	TankPtr	GetTankForPlayer( int inPlayerId );
	void	SpawnCatForPlayer( int inPlayerId );


private:
	Server();

	bool	InitNetworkManager();
	void	SetupWorld();

};