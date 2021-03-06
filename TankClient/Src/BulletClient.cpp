#include <TankClientPCH.h>

BulletClient::BulletClient()
{
	mSpriteComponent.reset( new SpriteComponent( this ) );
	mSpriteComponent->SetTexture( TextureManager::sInstance->GetTexture( "Bullet" ) );
}


void BulletClient::Read( InputMemoryBitStream& inInputStream )
{
	bool stateBit;

	inInputStream.Read( stateBit );
	if( stateBit )
	{
		Vector3 location;
		inInputStream.Read( location.mX );
		inInputStream.Read( location.mY );


		Vector3 velocity;
		inInputStream.Read( velocity.mX );
		inInputStream.Read( velocity.mY );
		SetVelocity( velocity );

		//dead reckon ahead by rtt, since this was spawned a while ago!
		SetLocation( location + velocity * NetworkManagerClient::sInstance->GetRoundTripTime() );


		float rotation;
		inInputStream.Read( rotation );
		SetRotation( rotation );
	}


	inInputStream.Read( stateBit );
	if( stateBit )
	{	
		Vector3 color;
		inInputStream.Read( color );
		SetColor( color );
	}

	inInputStream.Read( stateBit );
	if( stateBit )
	{	
		inInputStream.Read( mPlayerId, 8 );
	}

}

bool BulletClient::HandleCollisionWithTank( Tank* inTank )
{
	if( GetPlayerId() != inTank->GetPlayerId() )
	{
		RenderManager::sInstance->RemoveComponent( mSpriteComponent.get() );
	}
	return false;
}
