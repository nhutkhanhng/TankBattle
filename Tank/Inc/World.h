
class World
{

public:

	static void StaticInit();
	static std::unique_ptr< World >		sInstance;

	void AddGameObject( GameObjectPtr inGameObject );
	void RemoveGameObject( GameObjectPtr inGameObject );
	void Update();


	const std::vector< GameObjectPtr >&	GetGameObjects()	const	{ return mGameObjects; }

private:

	World();
	std::vector<GameObjectPtr> mGameObjects;

};