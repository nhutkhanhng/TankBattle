#include <TankServerPCH.h>

void ReplicationManagerTransmissionData::AddTransmission( int inNetworkId, ReplicationAction inAction, uint32_t inState )
{
	mTransmissions.emplace_back( inNetworkId, inAction, inState );
}

void ReplicationManagerTransmissionData::HandleDeliveryFailure( DeliveryNotificationManager* inDeliveryNotificationManager ) const
{
	for( const ReplicationTransmission& rt: mTransmissions )
	{
		int networkId = rt.GetNetworkId();

		switch( rt.GetAction() )
		{
		case RA_Create:
			HandleCreateDeliveryFailure( networkId );
			break;
		case RA_Update:
			HandleUpdateStateDeliveryFailure( networkId, rt.GetState(), inDeliveryNotificationManager );
			break;
		case RA_Destroy:
			HandleDestroyDeliveryFailure( networkId );
			break;
		}
		
	}
}

void ReplicationManagerTransmissionData::HandleDeliverySuccess( DeliveryNotificationManager* inDeliveryNotificationManager ) const
{
	for( const ReplicationTransmission& rt: mTransmissions )
	{
		switch( rt.GetAction() )
		{
		case RA_Create:
			HandleCreateDeliverySuccess( rt.GetNetworkId() );
			break;
		case RA_Destroy:
			HandleDestroyDeliverySuccess( rt.GetNetworkId() );
			break;
		}
	}
}




void ReplicationManagerTransmissionData::HandleCreateDeliveryFailure( int inNetworkId ) const
{
	GameObjectPtr gameObject = NetworkManagerServer::sInstance->GetGameObject( inNetworkId );
	if( gameObject )
	{
		mReplicationManagerServer->ReplicateCreate( inNetworkId, gameObject->GetAllStateMask() );
	}
}

void ReplicationManagerTransmissionData::HandleDestroyDeliveryFailure( int inNetworkId ) const
{
	mReplicationManagerServer->ReplicateDestroy( inNetworkId );
}

void ReplicationManagerTransmissionData::HandleUpdateStateDeliveryFailure( int inNetworkId, uint32_t inState, DeliveryNotificationManager* inDeliveryNotificationManager ) const
{
	//does the object still exist? it might be dead, in which case we don't resend an update
	if( NetworkManagerServer::sInstance->GetGameObject( inNetworkId ) )
	{
		for( const auto& inFlightPacket: inDeliveryNotificationManager->GetInFlightPackets() )
		{
			ReplicationManagerTransmissionDataPtr rmtdp = std::static_pointer_cast< ReplicationManagerTransmissionData >( inFlightPacket.GetTransmissionData( 'RPLM' ) );
					
			for( const ReplicationTransmission& otherRT: rmtdp->mTransmissions )
			{
				inState &= ~otherRT.GetState();
			}
		}
		
		if( inState )
		{
			mReplicationManagerServer->SetStateDirty( inNetworkId, inState );
		}
	}
}

void ReplicationManagerTransmissionData::HandleCreateDeliverySuccess( int inNetworkId ) const
{
	//we've received an ack for the create, so we can start sending as only an update
	mReplicationManagerServer->HandleCreateAckd( inNetworkId );
}

void ReplicationManagerTransmissionData::HandleDestroyDeliverySuccess( int inNetworkId ) const
{
	mReplicationManagerServer->RemoveFromReplication( inNetworkId );
}