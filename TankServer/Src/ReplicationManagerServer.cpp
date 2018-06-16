#include <TankServerPCH.h>

void ReplicationManagerServer::ReplicateCreate( int inNetworkId, uint32_t inInitialDirtyState )
{
	mNetworkIdToReplicationCommand[ inNetworkId ] = ReplicationCommand( inInitialDirtyState );
}

void ReplicationManagerServer::ReplicateDestroy( int inNetworkId )
{
	//it's broken if we don't find it...
	mNetworkIdToReplicationCommand[ inNetworkId ].SetDestroy();
}

void ReplicationManagerServer::RemoveFromReplication( int inNetworkId )
{
	mNetworkIdToReplicationCommand.erase( inNetworkId );
}

void ReplicationManagerServer::SetStateDirty( int inNetworkId, uint32_t inDirtyState )
{
	mNetworkIdToReplicationCommand[ inNetworkId ].AddDirtyState( inDirtyState );
}

void ReplicationManagerServer::HandleCreateAckd( int inNetworkId )
{
	mNetworkIdToReplicationCommand[ inNetworkId ].HandleCreateAckd();
}

void ReplicationManagerServer::Write( OutputMemoryBitStream& inOutputStream, ReplicationManagerTransmissionData* ioTransmissinData )
{
	//run through each replication command and do something...
	for( auto& pair: mNetworkIdToReplicationCommand )
	{
		ReplicationCommand& replicationCommand = pair.second;
		if( replicationCommand.HasDirtyState() )
		{
			int networkId = pair.first;

			inOutputStream.Write( networkId );

			//only need 2 bits for action...
			ReplicationAction action = replicationCommand.GetAction();
			inOutputStream.Write( action, 2 );

			uint32_t writtenState = 0;
			uint32_t dirtyState = replicationCommand.GetDirtyState();

			//now do what?
			switch( action )
			{
			case RA_Create:
				writtenState = WriteCreateAction( inOutputStream, networkId, dirtyState );
				break;
			case RA_Update:
				writtenState = WriteUpdateAction( inOutputStream, networkId, dirtyState );
				break;
			case RA_Destroy:
				writtenState = WriteDestroyAction( inOutputStream, networkId, dirtyState );
				break;
			}
		
			ioTransmissinData->AddTransmission( networkId, action, writtenState );

			//let's pretend everything was written- don't make this too hard
			// Lol =]]]
			replicationCommand.ClearDirtyState( writtenState );

		}
	}
}


uint32_t ReplicationManagerServer::WriteCreateAction( OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState )
{
	GameObjectPtr gameObject = NetworkManagerServer::sInstance->GetGameObject( inNetworkId );
	inOutputStream.Write( gameObject->GetClassId() );
	return gameObject->Write( inOutputStream, inDirtyState );
}

uint32_t ReplicationManagerServer::WriteUpdateAction( OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState )
{
	GameObjectPtr gameObject = NetworkManagerServer::sInstance->GetGameObject( inNetworkId );

	uint32_t writtenState = gameObject->Write( inOutputStream, inDirtyState );

	return writtenState;
}

uint32_t ReplicationManagerServer::WriteDestroyAction( OutputMemoryBitStream& inOutputStream, int inNetworkId, uint32_t inDirtyState )
{
	(void)inOutputStream;
	( void ) inNetworkId;
	( void ) inDirtyState;

	return inDirtyState;
}