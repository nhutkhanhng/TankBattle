#include <TankServerPCH.h>


BulletServer::BulletServer()
{
	//yarn lives a second...
	mTimeToDie = Timing::sInstance.GetFrameStartTime() + 3.f;
}

void BulletServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject( this );
}


void BulletServer::Update()
{
	Bullet::Update();

	if( Timing::sInstance.GetFrameStartTime() > mTimeToDie )
	{
		SetDoesWantToDie( true );
	}

}

bool BulletServer::HandleCollisionWithTank( Tank* inTank )
{
	if( inTank->GetPlayerId() != GetPlayerId() )
	{
		//kill yourself!
		SetDoesWantToDie( true );

		static_cast< TankServer* >( inTank )->TakeDamage( GetPlayerId() );
	}

	return false;
}


