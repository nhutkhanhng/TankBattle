#include <TankClientPCH.h>
#include <cassert>

void ReplicationManagerClient::Read( InputMemoryBitStream& inInputStream )
{
	while( inInputStream.GetRemainingBitCount() >= 32 )
	{
		//read the network id...
		int networkId; inInputStream.Read( networkId );
		
		//only need 2 bits for action...
		uint8_t action; inInputStream.Read( action, 2 );

		switch( action )
		{
		case RA_Create:
			ReadAndDoCreateAction( inInputStream, networkId );
			break;
		case RA_Update:
			ReadAndDoUpdateAction( inInputStream, networkId );
			break;
		case RA_Destroy:
			ReadAndDoDestroyAction( inInputStream, networkId );
			break;
		}

	}

}

void ReplicationManagerClient::ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
	//need 4 cc
	uint32_t fourCCName;
	inInputStream.Read( fourCCName );

	//could happen if our ack of the create got dropped so server resends create request 
	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
	if( !gameObject )
	{
		//create the object and map it...
		gameObject = GameObjectRegistry::sInstance->CreateGameObject( fourCCName );
		std::cout << "\nPosition of Texture Component in Client Side " << (*gameObject).GetLocation().mX << (*gameObject).GetLocation().mY << (*gameObject).GetClassId();
		gameObject->SetNetworkId( inNetworkId );
		NetworkManagerClient::sInstance->AddToNetworkIdToGameObjectMap( gameObject );
		
		assert( gameObject->GetClassId() == fourCCName );
	}

	//and read state
	gameObject->Read( inInputStream );
}

void ReplicationManagerClient::ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
	//need object
	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );

	gameObject->Read( inInputStream );
}

void ReplicationManagerClient::ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
	if( gameObject )
	{
		gameObject->SetDoesWantToDie( true );
		NetworkManagerClient::sInstance->RemoveFromNetworkIdToGameObjectMap( gameObject );
	}
}