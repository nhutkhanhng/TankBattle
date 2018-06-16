#include <TankPCH.h>

Bullet::Bullet() :
	mMuzzleSpeed( 3.f ),
	mVelocity( Vector3::Zero ),
	mPlayerId( 0 )
{
	SetScale( GetScale() * 0.25f );
	SetCollisionRadius( 0.125f );
}



uint32_t Bullet::Write( OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState ) const 
{
	uint32_t writtenState = 0;

	if( inDirtyState & EBRS_Pose )
	{
		inOutputStream.Write( (bool)true );

		Vector3 location = GetLocation();
		inOutputStream.Write( location.mX );
		inOutputStream.Write( location.mY );

		Vector3 velocity = GetVelocity();
		inOutputStream.Write( velocity.mX );
		inOutputStream.Write( velocity.mY );

		inOutputStream.Write( GetRotation() );

		writtenState |= EBRS_Pose;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}

	if( inDirtyState & EBRS_Color )
	{
		inOutputStream.Write( (bool)true );

		inOutputStream.Write( GetColor() );

		writtenState |= EBRS_Color;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}

	if( inDirtyState & EBRS_PlayerId )
	{
		inOutputStream.Write( (bool)true );

		inOutputStream.Write( mPlayerId, 8 );

		writtenState |= EBRS_PlayerId;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}




	return writtenState;
}



bool Bullet::HandleCollisionWithTank( Tank* inTank )
{
	( void ) inTank;

	return false;
}


void Bullet::InitFromShooter( Tank* inShooter )
{
	SetColor( inShooter->GetColor() );
	SetPlayerId( inShooter->GetPlayerId() );

	Vector3 forward = inShooter->GetForwardVector();
	SetVelocity( inShooter->GetVelocity() + forward * mMuzzleSpeed );
	SetLocation( inShooter->GetLocation() + forward * inShooter->GetCollisionRadius() * 1.5f );

	SetRotation( inShooter->GetRotation() );
}

namespace {
	bool CollideBox(const GameObject& A, const GameObject& B)
	{
		if ((A.GetLocation().mX + A.GetCollisionRadius() * A.GetScale() >= B.GetLocation().mX - B.GetCollisionRadius())* B.GetScale() &&
			(A.GetLocation().mX - A.GetCollisionRadius() * A.GetScale() <= B.GetLocation().mX + B.GetCollisionRadius())* B.GetScale() &&
			(A.GetLocation().mY + A.GetCollisionRadius() * A.GetScale() >= B.GetLocation().mY - B.GetCollisionRadius())* B.GetScale() &&
			(A.GetLocation().mY - A.GetCollisionRadius()* A.GetScale() <= B.GetLocation().mY + B.GetCollisionRadius() *B.GetScale())
			)
		{
			// set collision vector
			return true;
		}

		return false;
	}
}

// Interpolation
void Bullet::Update()
{
	
	float deltaTime = Timing::sInstance.GetDeltaTime();

	SetLocation( GetLocation() + mVelocity * deltaTime );

		float sourceRadius = GetCollisionRadius();
		Vector3 sourceLocation = GetLocation();

		for (auto goIt = World::sInstance->GetGameObjects().begin(), end = World::sInstance->GetGameObjects().end(); goIt != end; ++goIt)
		{
			GameObject* target = goIt->get();
			if (target != this && !target->DoesWantToDie())
			{
				if (CollideBox(*this, *target))
				{
					SetDoesWantToDie(true);
				}
			}
		}
}
