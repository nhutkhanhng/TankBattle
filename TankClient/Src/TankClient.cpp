#include <TankClientPCH.h>



TankClient::TankClient() :
	mTimeLocationBecameOutOfSync(0.f),
	mTimeVelocityBecameOutOfSync(0.f)
{
	mSpriteComponent.reset(new SpriteComponent(this));
	mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("Tank"));


	std::cout << "Texture " << mSpriteComponent->GetTexture()->GetHeight() << mSpriteComponent->GetTexture()->GetWidth();
}

void TankClient::HandleDying()
{
	Tank::HandleDying();

	//and if we're local, tell the hud so our health goes away!
	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		HUD::sInstance->SetPlayerHealth(0);
	}
}


void TankClient::Update()
{
	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		const Move* pendingMove = InputManager::sInstance->GetAndClearPendingMove();


		if (pendingMove) //is it time to sample a new move...
		{
			float deltaTime = pendingMove->GetDeltaTime();

			ProcessInput(deltaTime, pendingMove->GetInputState());
			SimulateMovement(deltaTime);
		}
	}
	else
	{
		SimulateMovement(Timing::sInstance.GetDeltaTime());

		if (TMath::Is2DVectorEqual(GetVelocity(), Vector3::Zero))
		{
			//we're in sync if our velocity is 0
			mTimeLocationBecameOutOfSync = 0.f;
		}
	}
}

void TankClient::Read(InputMemoryBitStream& inInputStream)
{
	bool stateBit;

	uint32_t readState = 0;

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		uint32_t playerId;
		inInputStream.Read(playerId);
		SetPlayerId(playerId);
		readState |= ECRS_PlayerId;
	}

	float oldRotation = GetRotation();
	Vector3 oldLocation = GetLocation();
	Vector3 oldVelocity = GetVelocity();

	float replicatedRotation;
	Vector3 replicatedLocation;
	Vector3 replicatedVelocity;

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		inInputStream.Read(replicatedVelocity.mX);
		inInputStream.Read(replicatedVelocity.mY);

		SetVelocity(replicatedVelocity);

		inInputStream.Read(replicatedLocation.mX);
		inInputStream.Read(replicatedLocation.mY);

		SetLocation(replicatedLocation);
		inInputStream.Read(replicatedRotation);
		std::cout << "\nReadRotation from server" << replicatedRotation;
		SetRotation(replicatedRotation);

		readState |= ECRS_Pose;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		inInputStream.Read(stateBit);
		mThrustDir = stateBit ? 1.f : -1.f;
	}
	else
	{
		mThrustDir = 0.f;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		Vector3 color;
		inInputStream.Read(color);
		SetColor(color);
		readState |= ECRS_Color;
	}

	inInputStream.Read(stateBit);
	if (stateBit)
	{
		mHealth = 0;
		inInputStream.Read(mHealth, 4);
		readState |= ECRS_Health;
	}

	if (GetPlayerId() == NetworkManagerClient::sInstance->GetPlayerId())
	{
		if ((readState & ECRS_Health) != 0)
		{
			HUD::sInstance->SetPlayerHealth(mHealth);
		}

		DoClientSidePredictionAfterReplicationForLocalTank(readState);

		if ((readState & ECRS_PlayerId) == 0)
		{
			InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, false);
		}
	}
	else
	{
		DoClientSidePredictionAfterReplicationForRemoteTank(readState);


		if ((readState & ECRS_PlayerId) == 0)
		{
			InterpolateClientSidePrediction(oldRotation, oldLocation, oldVelocity, true);
		}

	}


}




void TankClient::DoClientSidePredictionAfterReplicationForLocalTank(uint32_t inReadState)
{
	if ((inReadState & ECRS_Pose) != 0)
	{
		const MoveList& moveList = InputManager::sInstance->GetMoveList();

		for (const Move& move : moveList)
		{
			float deltaTime = move.GetDeltaTime();
			ProcessInput(deltaTime, move.GetInputState());

			SimulateMovement(deltaTime);
		}
	}
}


void TankClient::InterpolateClientSidePrediction(float inOldRotation, const Vector3& inOldLocation, const Vector3& inOldVelocity, bool inIsForRemoteTank)
{
	float roundTripTime = NetworkManagerClient::sInstance->GetRoundTripTime();

	if (!TMath::Is2DVectorEqual(inOldLocation, GetLocation()))
	{
		float time = Timing::sInstance.GetFrameStartTime();
		if (mTimeLocationBecameOutOfSync == 0.f)
		{
			mTimeLocationBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeLocationBecameOutOfSync;
		if (durationOutOfSync < roundTripTime)
		{
			SetLocation(Lerp(inOldLocation, GetLocation(), inIsForRemoteTank ? (durationOutOfSync / roundTripTime) : 0.1f));
		}
	}
	else
	{
		//sync
		mTimeLocationBecameOutOfSync = 0.f;
	}


	if (!TMath::Is2DVectorEqual(inOldVelocity, GetVelocity()))
	{
		float time = Timing::sInstance.GetFrameStartTime();
		if (mTimeVelocityBecameOutOfSync == 0.f)
		{
			mTimeVelocityBecameOutOfSync = time;
		}

		float durationOutOfSync = time - mTimeVelocityBecameOutOfSync;
		if (durationOutOfSync < roundTripTime)
		{
			SetVelocity(Lerp(inOldVelocity, GetVelocity(), inIsForRemoteTank ? (durationOutOfSync / roundTripTime) : 0.1f));
		}
	}
	else
	{
		//we're in sync
		mTimeVelocityBecameOutOfSync = 0.f;
	}

}

void TankClient::DoClientSidePredictionAfterReplicationForRemoteTank(uint32_t inReadState)
{
	if ((inReadState & ECRS_Pose) != 0)
	{
		float rtt = NetworkManagerClient::sInstance->GetRoundTripTime();
		

		// Crazy time ==]]]
		float deltaTime = 1.f / 30.f;

		while (true)
		{
			if (rtt < deltaTime)
			{
				SimulateMovement(rtt);
				break;
			}
			else
			{
				SimulateMovement(deltaTime);
				rtt -= deltaTime;
			}
		}
	}

}
