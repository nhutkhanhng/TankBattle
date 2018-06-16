#include <TankPCH.h>


std::unique_ptr<GameObjectRegistry>	GameObjectRegistry::sInstance;

void GameObjectRegistry::StaticInit()
{
	sInstance.reset(new GameObjectRegistry());
}

GameObjectRegistry::GameObjectRegistry()
{
}

void GameObjectRegistry::RegisterCreationFunction(uint32_t inFourCCName, GameObjectCreationFunc inCreationFunction)
{
	mCreationFunctionViaNameObject[inFourCCName] = inCreationFunction;
}

GameObjectPtr GameObjectRegistry::CreateGameObject(uint32_t inFourCCName)
{
	GameObjectCreationFunc creationFunc = mCreationFunctionViaNameObject[inFourCCName];
	GameObjectPtr gameObject = creationFunc();
	World::sInstance->AddGameObject(gameObject);

	return gameObject;
}