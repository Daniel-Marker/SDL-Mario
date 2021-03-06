#include "CharacterEnemy.h"

CharacterEnemy::CharacterEnemy(SDL_Renderer* renderer, std::string imagePath, Vector2D startPosition, LevelMap* map, float moveSpeed,
	float frameDelay, int noOfFrames, bool animating, int startFrame, int currentNumOfFrames,
	float initialJumpForce, float gravity, float jumpForceDecrement, float collisionRadius, bool screenWrapping, FACING facingDirection):
	Character(renderer, imagePath, startPosition, map, moveSpeed, initialJumpForce, gravity, jumpForceDecrement, collisionRadius,
		frameDelay, noOfFrames, animating, startFrame, currentNumOfFrames, screenWrapping, facingDirection), mAlive(true)
{
}

CharacterEnemy::~CharacterEnemy()
{
	if (mDeathSound && !mAlive)
	{
		mDeathSound->Play(0);
		SoundList::Instance()->AddSoundToList(mDeathSound);
	}
}

void CharacterEnemy::TakeDamage()
{
}

void CharacterEnemy::SetAlive(bool value)
{
	mAlive = value;
}

bool CharacterEnemy::GetAlive()
{
	return mAlive;
}
