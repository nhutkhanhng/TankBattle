#include <TankClientPCH.h>

BrickClient::BrickClient()
{
	mSpriteComponent.reset( new SpriteComponent( this ) );
	mSpriteComponent->SetTexture( TextureManager::sInstance->GetTexture( "Brick" ) );
}