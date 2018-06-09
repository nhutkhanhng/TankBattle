#include <TankPCH.h>

//zoom hardcoded at 100...if we want to lock players on screen, this could be calculated from zoom
const float HALF_WORLD_HEIGHT = 3.6f;
const float HALF_WORLD_WIDTH = 6.4f;

Tank::Tank() :
	GameObject(),
	mMaxRotationSpeed( 5.f ),
	mMaxLinearSpeed( 50.f ),
	mVelocity( Vector3::Zero ),
	mWallRestitution( 0.1f ),
	mCatRestitution( 0.1f ),
	mThrustDir( 0.f ),
	mPlayerId( 0 ),
	mIsShooting( false ),
	mHealth( 10 )
{
	SetCollisionRadius( 0.5f );
}

void Tank::ProcessInput( float inDeltaTime, const InputState& inInputState )
{
	float newRotation = GetRotation() + inInputState.GetDesiredHorizontalDelta() * mMaxRotationSpeed * inDeltaTime;
	SetRotation( newRotation );

	float inputForwardDelta = inInputState.GetDesiredVerticalDelta();
	mThrustDir = inputForwardDelta;

	mIsShooting = inInputState.IsShooting(); 
}

void Tank::AdjustVelocityByThrust( float inDeltaTime )
{
	Vector3 forwardVector = GetForwardVector();
	mVelocity = forwardVector * ( mThrustDir * inDeltaTime * mMaxLinearSpeed );
}

void Tank::SimulateMovement( float inDeltaTime )
{
	AdjustVelocityByThrust( inDeltaTime );
	SetLocation( GetLocation() + mVelocity * inDeltaTime );
	ProcessCollisions();
}

void Tank::Update()
{
	
}

void Tank::ProcessCollisions()
{
	//right now just bounce off the sides..
	ProcessCollisionsWithScreenWalls();

	float sourceRadius = GetCollisionRadius();
	Vector3 sourceLocation = GetLocation();

	for( auto goIt = World::sInstance->GetGameObjects().begin(), end = World::sInstance->GetGameObjects().end(); goIt != end; ++goIt )
	{
		GameObject* target = goIt->get();
		if( target != this && !target->DoesWantToDie() )
		{
			Vector3 targetLocation = target->GetLocation();
			float targetRadius = target->GetCollisionRadius();

			Vector3 delta = targetLocation - sourceLocation;
			float distSq = delta.LengthSq2D();
			float collisionDist = ( sourceRadius + targetRadius );
			if( distSq < ( collisionDist * collisionDist ) )
			{
				if( target->HandleCollisionWithTank( this ) )
				{

					Vector3 dirToTarget = delta;
					dirToTarget.Normalize2D();

					Vector3 acceptableDeltaFromSourceToTarget = dirToTarget * collisionDist;
					
					SetLocation( targetLocation - acceptableDeltaFromSourceToTarget );

					
					Vector3 relVel = mVelocity;
				
					//if other object is a cat, it might have velocity, so there might be relative velocity...
					Tank* targetCat = target->GetAsTank();
					if( targetCat )
					{
						relVel -= targetCat->mVelocity;
					}

					float relVelDotDir = Dot2D( relVel, dirToTarget );

					if (relVelDotDir > 0.f)
					{
						Vector3 impulse = relVelDotDir * dirToTarget;
					
						if( targetCat )
						{
							mVelocity -= impulse;
							mVelocity *= mCatRestitution;
						}
						else
						{
							mVelocity -= impulse * 2.f;
							mVelocity *= mWallRestitution;
						}

					}
				}
			}
		}
	}

}

void Tank::ProcessCollisionsWithScreenWalls()
{
	Vector3 location = GetLocation();
	float x = location.mX;
	float y = location.mY;

	float vx = mVelocity.mX;
	float vy = mVelocity.mY;

	float radius = GetCollisionRadius();

	if( ( y + radius ) >= HALF_WORLD_HEIGHT && vy > 0 )
	{
		mVelocity.mY = -vy * mWallRestitution;
		location.mY = HALF_WORLD_HEIGHT - radius;
		SetLocation( location );
	}
	else if( y <= ( -HALF_WORLD_HEIGHT - radius ) && vy < 0 )
	{
		mVelocity.mY = -vy * mWallRestitution;
		location.mY = -HALF_WORLD_HEIGHT - radius;
		SetLocation( location );
	}

	if( ( x + radius ) >= HALF_WORLD_WIDTH && vx > 0 )
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = HALF_WORLD_WIDTH - radius;
		SetLocation( location );
	}
	else if(  x <= ( -HALF_WORLD_WIDTH - radius ) && vx < 0 )
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = -HALF_WORLD_WIDTH - radius;
		SetLocation( location );
	}
}

uint32_t Tank::Write( OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState ) const
{
	uint32_t writtenState = 0;

	if( inDirtyState & ECRS_PlayerId )
	{
		inOutputStream.Write( (bool)true );
		inOutputStream.Write( GetPlayerId() );

		writtenState |= ECRS_PlayerId;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}


	if( inDirtyState & ECRS_Pose )
	{
		inOutputStream.Write( (bool)true );

		Vector3 velocity = mVelocity;
		inOutputStream.Write( velocity.mX );
		inOutputStream.Write( velocity.mY );

		Vector3 location = GetLocation();
		inOutputStream.Write( location.mX );
		inOutputStream.Write( location.mY );

		inOutputStream.Write( GetRotation() );

		writtenState |= ECRS_Pose;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}

	//always write mThrustDir- it's just two bits
	if( mThrustDir != 0.f )
	{
		inOutputStream.Write( true );
		inOutputStream.Write( mThrustDir > 0.f );
	}
	else
	{
		inOutputStream.Write( false );
	}

	if( inDirtyState & ECRS_Color )
	{
		inOutputStream.Write( (bool)true );
		inOutputStream.Write( GetColor() );

		writtenState |= ECRS_Color;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}

	if( inDirtyState & ECRS_Health )
	{
		inOutputStream.Write( (bool)true );
		inOutputStream.Write( mHealth, 4 );

		writtenState |= ECRS_Health;
	}
	else
	{
		inOutputStream.Write( (bool)false );
	}


	


	

	return writtenState;
	

}


