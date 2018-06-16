#include <TankPCH.h>

// Lock Zomm = 100
const float HALF_WORLD_HEIGHT = 3.6 - 0.64f;
const float HALF_WORLD_WIDTH = 6.4 - 0.68f;
Tank::Tank() :
	GameObject(),
	mMaxRotationSpeed( 5.f ),
	mMaxLinearSpeed( 50.f ),
	mVelocity( Vector3::Zero ),
	mWallRestitution( 0.1f ),
	mTankRestitution( 0.1f ),
	mThrustDir( 0.f ),
	mPlayerId( 0 ),
	mIsShooting( false ),
	mHealth( 10 )
{
	SetCollisionRadius( 0.3f );
}

namespace {

	bool double_equals(double a, double b, double epsilon = DBL_EPSILON)
	{
		return std::abs(a - b) < epsilon;
	}
}

void Tank::ProcessInput( float inDeltaTime, const InputState& inInputState )
{
	/*std::cout << "GetDesirdHorizontalDelta" << inInputState.GetDesiredHorizontalDelta() << ","<< inInputState.GetDesiredVerticalDelta();*/

	float Direction = 0.f;

	float inputForwardDelta = 0.f;

	if ( (double_equals(inInputState.GetDesiredHorizontalDelta(), 0.f) 
		&& double_equals(inInputState.GetDesiredVerticalDelta(), 0.f))
													== false)
	{
		// float newRotation = GetRotation() + inInputState.GetDesiredHorizontalDelta() * mMaxRotationSpeed * inDeltaTime;
		//std::cout << "\nHorizontalDela()" << inInputState.GetDesiredHorizontalDelta();
		std::cout << "\nGetDesiredVerticalDelta()" << inInputState.GetDesiredVerticalDelta();

		float Direction = ((inInputState.GetDesiredVerticalDelta() > 0) ? 0.f : 3.14159f);

		if (inInputState.GetDesiredHorizontalDelta() != 0)
			Direction = inInputState.GetDesiredHorizontalDelta() * 1.5708f;

		//if (inInputState.GetDesiredVerticalDelta() < 0.f)
		//	Direction *= -1.f;

		// inInputState.GetDesiredHorizontalDelta() * 1.5f + inInputState.GetDesiredVerticalDelta() * 3.f


		if (GetRotation() == Direction)
			inputForwardDelta = 1.f;

		//std::cout << "\nDirection : " << Direction;

		SetRotation(Direction);

		std::cout << "\nLocation" << GetLocation();
	}

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

namespace Collision {
	bool CollideBox(const GameObject& A, const GameObject& B)
	{
		if ((A.GetLocation().mX + A.GetCollisionRadius() >= B.GetLocation().mX - B.GetCollisionRadius()) &&
			(A.GetLocation().mX - A.GetCollisionRadius() <= B.GetLocation().mX + B.GetCollisionRadius()) &&
			(A.GetLocation().mY + A.GetCollisionRadius() >= B.GetLocation().mY - B.GetCollisionRadius()) &&
			(A.GetLocation().mY - A.GetCollisionRadius() <= B.GetLocation().mY + B.GetCollisionRadius())
			)
		{
			// set collision vector
			return true;
		}

		return false;
	}


	bool CollideAABB(GameObject &A, const GameObject &B)
	{

		return false;
	}
}
void Tank::ProcessCollisions()
{
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

			Tank* targetTank = target->GetAsTank();
			Vector3 Vdiff;

			if (targetTank)
				Vdiff = this->GetVelocity() - targetTank->GetVelocity();
			else
				Vdiff = this->GetVelocity();


			Vdiff.Normalize2D();

			float CUV = Dot2D(delta,Vdiff);

			if (Collision::CollideBox(*this, *target))
			{
				if (target->HandleCollisionWithTank(this))
				{

					Vector3 dirToTarget = delta;

					/*dirToTarget.Normalize2D();*/

					

					std::cout << "DirToTarget" << dirToTarget;
					std::cout << "Vecloctiy" << GetVelocity();

					//if (GetForwardVector().Length() > 0)
					//	dirToTarget = dirToTarget * GetForwardVector() * GetForwardVector();

					std::cout << "\nDirToTarget" << dirToTarget;

					Vector3 acceptableDeltaFromSourceToTarget;

				/*	if (GetForwardVector().Length() > 0)
						acceptableDeltaFromSourceToTarget = dirToTarget * collisionDist * GetForwardVector() * GetForwardVector();
					else
						acceptableDeltaFromSourceToTarget = dirToTarget * collisionDist;*/

					acceptableDeltaFromSourceToTarget = dirToTarget /** collisionDist*/;

					std::cout << "\nDirToTarget" << acceptableDeltaFromSourceToTarget;

					/*acceptableDeltaFromSourceToTarget = acceptableDeltaFromSourceToTarget*  GetForwardVector();*/

					std::cout << "\nAccetable " << acceptableDeltaFromSourceToTarget;


					//SetLocation(targetLocation - acceptableDeltaFromSourceToTarget/* - sourceLocation*/);
					/*SetLocation(sourceLocation - acceptableDeltaFromSourceToTarget);*/
					if (this->GetVelocity().mY == 0.f)
					{
						acceptableDeltaFromSourceToTarget.mY = 0.f;
					}
					else
						acceptableDeltaFromSourceToTarget.mX = 0.f;

					float CUV = Dot2D(sourceLocation, targetLocation);

					std::cout << " Position " << GetLocation();
					SetLocation(sourceLocation - acceptableDeltaFromSourceToTarget * mTankRestitution/** (collisionDist * distSq)*/ /* - sourceLocation*/);
					std::cout << " Position " << GetLocation();

					Vector3 relVel = mVelocity;

					//if other object is a cat, it might have velocity, so there might be relative velocity...
					Tank* targetCat = target->GetAsTank();
					if (targetCat)
					{
						relVel -= targetCat->mVelocity;
					}

					float relVelDotDir = Dot2D(relVel, dirToTarget);

					if (relVelDotDir > 0.f)
					{
						Vector3 impulse = relVelDotDir * dirToTarget;

						if (targetCat)
						{
							mVelocity -= impulse;
							mVelocity *= mTankRestitution;
						}
						else
						{
							mVelocity -= impulse * 2.f;
							mVelocity *= mWallRestitution;
						}

					}

					//Vdiff = Vdiff * (1 / acceptableDeltaFromSourceToTarget.Length2D());
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
	else if( y <= ( -HALF_WORLD_HEIGHT + radius ) && vy < 0 )
	{
		mVelocity.mY = -vy * mWallRestitution;
		location.mY = -HALF_WORLD_HEIGHT + radius;
		SetLocation( location );
	}

	if( ( x + radius ) >= HALF_WORLD_WIDTH && vx > 0 )
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = HALF_WORLD_WIDTH - radius;
		SetLocation( location );
	}
	else if(  x <= ( -HALF_WORLD_WIDTH + radius ) && vx < 0 )
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = -HALF_WORLD_WIDTH + radius;
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

		/*std::cout << "\nWriteRotation" << GetRotation();*/
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


