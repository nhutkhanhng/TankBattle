#include <TankServerPCH.h>


BrickServer::BrickServer()
{
}

void BrickServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject( this );
}


bool BrickServer::HandleCollisionWithTank( Tank* inTank )
{
	//kill yourself!
	/*SetDoesWantToDie( true );*/

	/*ScoreBoardManager::sInstance->IncScore( inTank->GetPlayerId(), 1 );*/

	return true;
}


