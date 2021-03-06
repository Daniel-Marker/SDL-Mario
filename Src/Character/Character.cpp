#include "Character.h"
#include "Texture2D.h"
#include "Constants.h"
#include "CharacterPlayable.h"

Character::Character(SDL_Renderer* renderer, std::string imagePath, Vector2D startPosition, LevelMap* map, float moveSpeed, float initialJumpForce, float gravity, float jumpForceDecrement, float collisionRadius,
	float frameDelay, int noOfFrames, bool animating, int currentStartFrame, int currentNumOfFrames, bool screenWrappingEnabled, FACING facingDirection) :
	mRenderer(renderer), mPosition(startPosition), mFacingDirection(facingDirection),
	mMovingLeft(false), mMovingRight(false), mJumping(false), mCanJump(false), mCollisionRadius(collisionRadius),
	mCurrentLevelMap(map), mJumpForce(0.0f), mMovementSpeed(moveSpeed), cInitialJumpForce(initialJumpForce), cGravity(gravity), cJumpForceDecrement(jumpForceDecrement),
	mFrameDelay(frameDelay), mFrameDelayTimer(frameDelay), mNumFrames(noOfFrames), mAnimating(animating), mCurrentFrame(0),
	mSingleSpriteWidth(0), mSingleSpriteHeight(0), mCurrentStartFrame(currentStartFrame), mCurrentNumOfFrames(currentNumOfFrames), mScreenWrappingEnabled(screenWrappingEnabled)
{
	mTexture = new Texture2D(mRenderer);
	mTexture->LoadFromFile(imagePath);

	mSingleSpriteWidth = mTexture->GetWidth() / noOfFrames;
	mSingleSpriteHeight = mTexture->GetHeight();
}

Character::~Character()
{
	mRenderer = NULL;

	delete mTexture;
	mTexture = NULL;

	if(mJumpSound)
		delete mJumpSound;
}

void Character::Render(int xOffset)
{
	int left = mCurrentFrame * mSingleSpriteWidth;

	//Get the portion of the spritesheet you want to draw
	//								{XPos, YPos, WidthOfSingleSprite, HeightOfSingleSprite}
	SDL_Rect portionOfSpriteSheet = { left, 0, mSingleSpriteWidth, mSingleSpriteHeight };
	SDL_Rect destRect = { (int)(mPosition.x + xOffset), (int)(mPosition.y), mSingleSpriteWidth, mSingleSpriteHeight };

	if (mFacingDirection == FACING_RIGHT)
		mTexture->Render(portionOfSpriteSheet, destRect, SDL_FLIP_NONE);
	else
		mTexture->Render(portionOfSpriteSheet, destRect, SDL_FLIP_HORIZONTAL);
}

void Character::Update(float deltaTime, SDL_Event e)
{
	int footPosition = (int)(mPosition.y + mSingleSpriteHeight) / mCurrentLevelMap->GetTileset().tileHeight;
	int headPosition = (int)(mPosition.y) / mCurrentLevelMap->GetTileset().tileHeight;
	int centralXPosition = (mPosition.x + (mSingleSpriteWidth * 0.5f)) / mCurrentLevelMap->GetTileset().tileWidth;
	int leftXPosition = (mPosition.x + mSingleSpriteWidth * GRAVITY_CHECK_FALLOFF) / mCurrentLevelMap->GetTileset().tileWidth;
	int rightXPosition = (mPosition.x + mSingleSpriteWidth * (1.0f - GRAVITY_CHECK_FALLOFF)) / mCurrentLevelMap->GetTileset().tileWidth;

	//Deal with jumping first
	if (mJumping)
	{
		if(mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(headPosition, centralXPosition)) &&
			mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(headPosition, leftXPosition)) &&
			mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(headPosition, rightXPosition)))
		{

			//Adjust the position
			mPosition.y -= mJumpForce * deltaTime;

			//Reduce the jump force
			mJumpForce -= cJumpForceDecrement * deltaTime;

			//Has the jump force reduced to zero?
			if (mJumpForce <= 0.0f)
				mJumping = false;
		}
		else
			HitTile();
	}

	if (mMovingLeft)
		MoveLeft(deltaTime);
	else if (mMovingRight)
		MoveRight(deltaTime);

	if (mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(footPosition, leftXPosition)) &&
		mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(footPosition, centralXPosition)) && 
		mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(footPosition, rightXPosition)))
		AddGravity(deltaTime);
	else
		mCanJump = true;
	//Collided with ground so we can jump again

	if (mAnimating)
	{
		mFrameDelayTimer -= deltaTime;
		if (mFrameDelayTimer <= 0.0f)
		{
			//Reset frame delay count
			mFrameDelayTimer = mFrameDelay;
			mCurrentFrame++;

			//Loop frame around if it goes beyond the number of frames
			if (mCurrentFrame > mCurrentStartFrame + mCurrentNumOfFrames - 1)
				mCurrentFrame = mCurrentStartFrame;

			//Skip ahead if further behind than intended
			if (mCurrentFrame < mCurrentStartFrame)
				mCurrentFrame = mCurrentStartFrame;
		}
	}
}

void Character::OnPlayerCollision(CharacterPlayable* player)
{
}

void Character::SetPosition(Vector2D newPosition)
{
	mPosition = newPosition;
}

Vector2D Character::GetPosition()
{
	return mPosition;
}

float Character::GetCollisionRadius()
{
	return mCollisionRadius;
}

Rect2D Character::GetCollisionBox()
{
	return Rect2D(mPosition.x, mPosition.y, mSingleSpriteWidth, mSingleSpriteHeight);
}

Circle2D Character::GetCollisionCircle()
{
	return Circle2D(mPosition.x + mSingleSpriteWidth / 2.0f, mPosition.y + mSingleSpriteHeight / 2.0f, mCollisionRadius);
	//Adds half the sprite width so that the centre of the circle returned is the centre of the object
}

bool Character::IsJumping()
{
	return mJumping;
}

void Character::CancelJump()
{
	mJumping = false;
	mCanJump = true;
}

void Character::MoveLeft(float deltaTime)
{
	int xPos = (int)(roundf(mPosition.x)) / mCurrentLevelMap->GetTileset().tileWidth;
	int yPos = (int)(roundf((mPosition.y + mSingleSpriteHeight * 0.25f) / (float)mCurrentLevelMap->GetTileset().tileHeight));
	//+1/4 of sprite height so that the player can still fit through small gaps (as seen at the end of level 2)

	mFacingDirection = FACING_LEFT;
	if (mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(yPos, xPos)))
	{
		mPosition.x -= mMovementSpeed * deltaTime;
	}

	//if character is offscreen, then wrap around
	if (mPosition.x < 0)
		ScreenWrap(deltaTime);
}

void Character::MoveRight(float deltaTime)
{
	int xPos = (int)(roundf(mPosition.x + mSingleSpriteWidth)) / mCurrentLevelMap->GetTileset().tileWidth;
	int yPos = (int)(roundf((mPosition.y + mSingleSpriteHeight * 0.25f) / (float)mCurrentLevelMap->GetTileset().tileHeight));
	//+1/4 of sprite height so that the player can still fit through small gaps (as seen at the end of level 2)

	mFacingDirection = FACING_RIGHT;
	if (mCurrentLevelMap->TileIsPassable(mCurrentLevelMap->GetTileAt(yPos, xPos)))
	{
		mPosition.x += mMovementSpeed * deltaTime;
	}

	//if half of character is off screen, then wrap around
	if (mPosition.x + mSingleSpriteWidth * 0.5f > SCREEN_WIDTH)
		ScreenWrap(deltaTime);
}

void Character::Jump()
{
	if (!mJumping && mCanJump)
	{
		mJumpForce = cInitialJumpForce;
		mJumping = true;
		mCanJump = false;

		if(mJumpSound)
			mJumpSound->Play(0);
	}
}

void Character::AddGravity(float deltaTime)
{
	mPosition.y += cGravity * deltaTime;
	mCanJump = false;

	if (mPosition.y > SCREEN_HEIGHT - mSingleSpriteHeight)
	{
		//at the bottom of the screen, so stop moving the character
		mPosition.y = SCREEN_HEIGHT - mSingleSpriteHeight;
		mCanJump = true;
	}
}

void Character::HitTile()
{
	mJumping = false;
}

void Character::ScreenWrap(float deltaTime)
{
	if (mScreenWrappingEnabled)
	{
		if (mPosition.x < -mSingleSpriteWidth)
			mPosition.x = SCREEN_WIDTH - mSingleSpriteWidth * 0.5f;
		else if (mPosition.x + mSingleSpriteWidth * 0.5f > SCREEN_WIDTH)
			mPosition.x = -mSingleSpriteWidth;

		//subtract gravity as characters fall slightly when reaching the edge of the screen
		mPosition.y -= cGravity * deltaTime;
	}
}
