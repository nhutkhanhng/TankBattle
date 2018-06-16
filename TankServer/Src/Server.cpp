
#include <TankServerPCH.h>

const float HALF_WORLD_HEIGHT = 3.6f;
const float HALF_WORLD_WIDTH = 6.4f;

bool Server::StaticInit()
{
	sInstance.reset( new Server() );

	return true;
}

Server::Server()
{

	GameObjectRegistry::sInstance->RegisterCreationFunction( 'TANK', TankServer::StaticCreate );
	GameObjectRegistry::sInstance->RegisterCreationFunction( 'BRIK', BrickServer::StaticCreate );
	GameObjectRegistry::sInstance->RegisterCreationFunction( 'BULT', BulletServer::StaticCreate );

	InitNetworkManager();
	
	//NetworkManagerServer::sInstance->SetDropPacketChance( 0.8f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.25f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.5f );
	//NetworkManagerServer::sInstance->SetSimulatedLatency( 0.1f );

}


int Server::Run()
{
	SetupWorld();

	return Engine::Run();
}

bool Server::InitNetworkManager()
{
	string portString = StringUtils::GetCommandLineArg( 1 );
	uint16_t port = stoi( portString );

	return NetworkManagerServer::StaticInit( port );
}


namespace
{
	
	void CreateMap()
	{
		GameObjectPtr go;
		Vector3 Location;

		int row = 10;
		int column = 15;


		int count = 0;
		for (int i = 1; i <= row; i++)
		{
			for (int j = 1; j <= column; j++)
			{		
				if (i % 4 == 0 && j % 3 != 0)
				{
					if (count > 20)
						return;

					go = GameObjectRegistry::sInstance->CreateGameObject('BRIK');
					Location.mY = -HALF_WORLD_HEIGHT + 0.64f * (i + 1) - 0.32;
					Location.mX = -HALF_WORLD_WIDTH + 0.64f * (j + 1) - 0.32;

					go->SetLocation(Location);

					count++;
				}
			}
		}
		

	}

	void CreateRowBrick()
	{
		GameObjectPtr go;

		float i = -HALF_WORLD_WIDTH + 0.64;

		while (i <= HALF_WORLD_WIDTH - 0.96)
		{
			go = GameObjectRegistry::sInstance->CreateGameObject('BRIK');
			/*Vector3 Location = TMath::GetRandomVector( MinLocation, MaxLocation );*/
			Vector3 Location;

			Location.mX = i + 0.32f;
			Location.mY = -HALF_WORLD_HEIGHT + 0.32f;


			go->SetLocation(Location);

			go = GameObjectRegistry::sInstance->CreateGameObject('BRIK');
			Location.mY = HALF_WORLD_HEIGHT - 0.32f;
			go->SetLocation(Location);

			i += 0.64f;
			
		}	
	}
}


void Server::SetupWorld()
{
	//spawn some random mice
	CreateMap();
	/*CreateColumnBrick();*/



	/*CreateBrick(10, false);*/
	//
	////spawn more random mice!
	//CreateRandomMice( 10 );
}

void Server::DoFrame()
{
	NetworkManagerServer::sInstance->ProcessIncomingPackets();

	NetworkManagerServer::sInstance->CheckForDisconnects();

	NetworkManagerServer::sInstance->RespawnCats();

	Engine::DoFrame();

	NetworkManagerServer::sInstance->SendOutgoingPackets();

}

void Server::HandleNewClient( ClientProxyPtr inClientProxy )
{
	
	int playerId = inClientProxy->GetPlayerId();
	
	ScoreBoardManager::sInstance->AddEntry( playerId, inClientProxy->GetName() );
	SpawnCatForPlayer( playerId );
}

void Server::SpawnCatForPlayer( int inPlayerId )
{
	TankPtr tank = std::static_pointer_cast< Tank >( GameObjectRegistry::sInstance->CreateGameObject( 'TANK' ) );
	tank->SetColor( ScoreBoardManager::sInstance->GetEntry( inPlayerId )->GetColor() );
	tank->SetPlayerId( inPlayerId );
	//gotta pick a better spawn location than this...
	tank->SetLocation( Vector3( 1.f - static_cast< float >( inPlayerId ), 0.f, 0.f ) );

}

void Server::HandleLostClient( ClientProxyPtr inClientProxy )
{
	//kill client's cat
	//remove client from scoreboard
	int playerId = inClientProxy->GetPlayerId();

	ScoreBoardManager::sInstance->RemoveEntry( playerId );
	TankPtr tank = GetTankForPlayer( playerId );
	if( tank )
	{
		tank->SetDoesWantToDie( true );
	}
}

TankPtr Server::GetTankForPlayer( int inPlayerId )
{

	const auto& gameObjects = World::sInstance->GetGameObjects();
	for( int i = 0, c = gameObjects.size(); i < c; ++i )
	{
		GameObjectPtr go = gameObjects[ i ];
		Tank* tank = go->GetAsTank();
		if( tank && tank->GetPlayerId() == inPlayerId )
		{
			return std::static_pointer_cast< Tank >( go );
		}
	}

	return nullptr;

}