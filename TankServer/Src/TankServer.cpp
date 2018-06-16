#include <TankServerPCH.h>

TankServer::TankServer() :
	mTankControlType( ETCT_Human ),
	mTimeOfNextShot( 0.f ),
	mTimeBetweenShots( 0.2f )
{}

void TankServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject( this );
}

void TankServer::Update()
{
	Tank::Update();
	
	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();
	float oldRotation = GetRotation();

	if( mTankControlType == ETCT_Human )
	{
		ClientProxyPtr client = NetworkManagerServer::sInstance->GetClientProxy( GetPlayerId() );
		if( client )
		{
			MoveList& moveList = client->GetUnprocessedMoveList();
			for( const Move& unprocessedMove : moveList )
			{
				const InputState& currentState = unprocessedMove.GetInputState();

				float deltaTime = unprocessedMove.GetDeltaTime();

				ProcessInput( deltaTime, currentState );
				SimulateMovement( deltaTime );

				//LOG( "Server Move Time: %3.4f deltaTime: %3.4f left rot at %3.4f", unprocessedMove.GetTimestamp(), deltaTime, GetRotation() );

			}

			moveList.Clear();
		}
	}
	else
	{
		//do some AI stuff
		SimulateMovement( Timing::sInstance.GetDeltaTime() );
	}

	HandleShooting();

	if( !TMath::Is2DVectorEqual( oldLocation, GetLocation() ) ||
		!TMath::Is2DVectorEqual( oldVelocity, GetVelocity() ) ||
		oldRotation != GetRotation() )
	{
		NetworkManagerServer::sInstance->SetStateDirty( GetNetworkId(), ECRS_Pose );
	}

}

void TankServer::HandleShooting()
{
	float time = Timing::sInstance.GetFrameStartTime();
	if( mIsShooting && Timing::sInstance.GetFrameStartTime() > mTimeOfNextShot )
	{
		//not exact, but okay
		mTimeOfNextShot = time + mTimeBetweenShots;

		//fire!
		BulletPtr bullet = std::static_pointer_cast< Bullet >( GameObjectRegistry::sInstance->CreateGameObject( 'BULT' ) );
		bullet->InitFromShooter( this );
	}
}

void TankServer::TakeDamage( int inDamagingPlayerId )
{
	mHealth--;
	if( mHealth <= 0.f )
	{
		ScoreBoardManager::sInstance->IncScore( inDamagingPlayerId, 1 );
		SetDoesWantToDie( true );

		ClientProxyPtr clientProxy = NetworkManagerServer::sInstance->GetClientProxy( GetPlayerId() );

		if( clientProxy )
		{
			clientProxy->HandleTankDied();
		}
	}

	//tell the world our health dropped
	NetworkManagerServer::sInstance->SetStateDirty( GetNetworkId(), ECRS_Health );
}
