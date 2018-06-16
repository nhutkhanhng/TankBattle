
#include <TankClientPCH.h>

bool Client::StaticInit( )
{
	Client* client = new Client();

	if( WindowManager::StaticInit() == false )
	{
		return false;
	}
	
	if( GraphicsDriver::StaticInit( WindowManager::sInstance->GetMainWindow() ) == false )
	{
		return false;
	}

	TextureManager::StaticInit();
	RenderManager::StaticInit();
	InputManager::StaticInit();

	HUD::StaticInit();

	sInstance.reset( client );

	return true;
}

Client::Client()
{
	GameObjectRegistry::sInstance->RegisterCreationFunction( 'TANK', TankClient::StaticCreate );
	GameObjectRegistry::sInstance->RegisterCreationFunction( 'BRIK', BrickClient::StaticCreate );
	GameObjectRegistry::sInstance->RegisterCreationFunction( 'BULT', BulletClient::StaticCreate );

	string destination = StringUtils::GetCommandLineArg( 1 );
	string name = StringUtils::GetCommandLineArg( 2 );

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString( destination );

	NetworkManagerClient::StaticInit( *serverAddress, name );

	/*NetworkManagerClient::sInstance->SetDropPacketChance( 0.6f );*/
	//NetworkManagerClient::sInstance->SetSimulatedLatency( 0.25f );
	//NetworkManagerClient::sInstance->SetSimulatedLatency( 0.5f );
	/*NetworkManagerClient::sInstance->SetSimulatedLatency( 0.1f );*/
}



void Client::DoFrame()
{
	InputManager::sInstance->Update();

	Engine::DoFrame();

	NetworkManagerClient::sInstance->ProcessIncomingPackets();

	RenderManager::sInstance->Render();

	NetworkManagerClient::sInstance->SendOutgoingPackets();
}

void Client::HandleEvent( SDL_Event* inEvent )
{
	switch( inEvent->type )
	{
	case SDL_KEYDOWN:
		InputManager::sInstance->HandleInput( EIA_Pressed, inEvent->key.keysym.sym );
		break;
	case SDL_KEYUP:
		InputManager::sInstance->HandleInput( EIA_Released, inEvent->key.keysym.sym );
		break;
	default:
		break;
	}
}

