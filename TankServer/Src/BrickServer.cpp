#include <TankServerPCH.h>


BrickServer::BrickServer()
{
}

void BrickServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject( this );
}


bool BrickServer::HandleCollisionWithTank( Tank* inCat )
{
	//kill yourself!
	SetDoesWantToDie( true );

	ScoreBoardManager::sInstance->IncScore( inCat->GetPlayerId(), 1 );

	return false;
}


