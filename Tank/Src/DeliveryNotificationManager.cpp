#include "TankPCH.h"

namespace
{
	const float kDelayBeforeAckTimeout = 0.5f;
}

DeliveryNotificationManager::DeliveryNotificationManager( bool inShouldSendAcks, bool inShouldProcessAcks ) :
mNextOutgoingSequenceNumber( 0 ),
mNextExpectedSequenceNumber( 0 ),
//everybody starts at 0...
mShouldSendAcks( inShouldSendAcks ),
mShouldProcessAcks( inShouldProcessAcks ),
mDeliveredPacketCount( 0 ),
mDroppedPacketCount( 0 ),
mDispatchedPacketCount( 0 )
{
}


//we're going away- log how well we did...
DeliveryNotificationManager::~DeliveryNotificationManager()
{
	LOG( "DNM destructor. Delivery rate %d%%, Drop rate %d%%",
		( 100 * mDeliveredPacketCount ) / mDispatchedPacketCount,
		( 100 * mDroppedPacketCount ) / mDispatchedPacketCount );
}



InFlightPacket* DeliveryNotificationManager::WriteSequenceNumber( OutputMemoryBitStream& inOutputStream )
{
	//write the sequence number, but also create an inflight packet for this...
	PacketSequenceNumber sequenceNumber = mNextOutgoingSequenceNumber++;
	inOutputStream.Write( sequenceNumber );

	++mDispatchedPacketCount;

	if( mShouldProcessAcks )
	{
		mInFlightPackets.emplace_back( sequenceNumber );

		return &mInFlightPackets.back();
	}
	else
	{
		return nullptr;
	}
}

void DeliveryNotificationManager::WriteAckData( OutputMemoryBitStream& inOutputStream )
{
	bool hasAcks = ( mPendingAcks.size() > 0 );

	inOutputStream.Write( hasAcks );
	if( hasAcks )
	{
		//note, we could write all the acks
		mPendingAcks.front().Write( inOutputStream );
		mPendingAcks.pop_front();
	}
}



bool DeliveryNotificationManager::ProcessSequenceNumber( InputMemoryBitStream& inInputStream )
{
	PacketSequenceNumber	sequenceNumber;

	inInputStream.Read( sequenceNumber );
	if( sequenceNumber == mNextExpectedSequenceNumber )
	{
		mNextExpectedSequenceNumber = sequenceNumber + 1;
		//is this what we expect? great, let's add an ack to our pending list
		// =]]]]
		if( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		return true;
	}

	else if( sequenceNumber < mNextExpectedSequenceNumber )
	{
		return false;
	}
	else if( sequenceNumber > mNextExpectedSequenceNumber )
	{
		
		mNextExpectedSequenceNumber = sequenceNumber + 1;
	
		if( mShouldSendAcks )
		{
			AddPendingAck( sequenceNumber );
		}
		return true;
	}

	//drop packet if we couldn't even read sequence number!
	return false;
}


void DeliveryNotificationManager::ProcessAcks( InputMemoryBitStream& inInputStream )
{

	bool hasAcks;
	inInputStream.Read( hasAcks );
	if( hasAcks )
	{
		AckRange ackRange;
		ackRange.Read( inInputStream );

		
		PacketSequenceNumber nextAckdSequenceNumber = ackRange.GetStart();
		uint32_t onePastAckdSequenceNumber = nextAckdSequenceNumber + ackRange.GetCount();
		while( nextAckdSequenceNumber < onePastAckdSequenceNumber && !mInFlightPackets.empty() )
		{
			const auto& nextInFlightPacket = mInFlightPackets.front();
			//if the packet has a lower sequence number, we didn't get an ack for it, so it probably wasn't delivered
			PacketSequenceNumber nextInFlightPacketSequenceNumber = nextInFlightPacket.GetSequenceNumber();
			if( nextInFlightPacketSequenceNumber < nextAckdSequenceNumber )
			{
				auto copyOfInFlightPacket = nextInFlightPacket;
				mInFlightPackets.pop_front();
				HandlePacketDeliveryFailure( copyOfInFlightPacket );
			}
			else if( nextInFlightPacketSequenceNumber == nextAckdSequenceNumber )
			{
				HandlePacketDeliverySuccess( nextInFlightPacket );
				//received!
				mInFlightPackets.pop_front();

				++nextAckdSequenceNumber;
			}
			else if( nextInFlightPacketSequenceNumber > nextAckdSequenceNumber )
			{
				//we've already ackd some packets in here.
				//keep this packet in flight, but keep going through the ack...
				++nextAckdSequenceNumber;
			}
		}
	}
}

void DeliveryNotificationManager::ProcessTimedOutPackets()
{
	float timeoutTime = Timing::sInstance.GetTimef() - kDelayBeforeAckTimeout;

	while( !mInFlightPackets.empty() )
	{
		const auto& nextInFlightPacket = mInFlightPackets.front();

		//was this packet dispatched before the current time minus the timeout duration?
		if( nextInFlightPacket.GetTimeDispatched() < timeoutTime )
		{
			//it failed! let us know about that
			HandlePacketDeliveryFailure( nextInFlightPacket );
			mInFlightPackets.pop_front();
		}
		else
		{
			//it wasn't, and packets are all in order by time here, so we know we don't have to check farther
			break;
		}
	}
}

void DeliveryNotificationManager::AddPendingAck( PacketSequenceNumber inSequenceNumber )
{
	if( mPendingAcks.size() == 0 || !mPendingAcks.back().ExtendIfShould( inSequenceNumber ) )
	{
		mPendingAcks.emplace_back( inSequenceNumber );
	}
}


void DeliveryNotificationManager::HandlePacketDeliveryFailure( const InFlightPacket& inFlightPacket )
{
	++mDroppedPacketCount;
	inFlightPacket.HandleDeliveryFailure( this );
}


void DeliveryNotificationManager::HandlePacketDeliverySuccess( const InFlightPacket& inFlightPacket )
{
	++mDeliveredPacketCount;
	inFlightPacket.HandleDeliverySuccess( this );
}
