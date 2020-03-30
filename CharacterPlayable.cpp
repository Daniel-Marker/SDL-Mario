#include "CharacterPlayable.h"

CharacterPlayable::CharacterPlayable(SDL_Renderer* renderer, std::string imagePath, Vector2D startPosition, int jumpKey, int rightKey, int leftKey,
	LevelMap* map, float moveSpeed, std::vector<CharacterEnemy*>* const enemiesList, std::string name, float scoreXPos, int initialLives):
	Character(renderer, imagePath, startPosition, map, moveSpeed), mState(IDLE), mScore(0), mEnemiesList(enemiesList), mName(name), mScoreXPos(scoreXPos), mLives(initialLives)
{
	mInputMap[JUMP] = jumpKey;
	mInputMap[RIGHT] = rightKey;
	mInputMap[LEFT] = leftKey;

	mJumpSound = new SoundEffect();
	mJumpSound->Load("SFX/jumpSound.ogg");
}

void CharacterPlayable::Render()
{
	if (!mInvulnerable)
	{
		if (mFacingDirection == FACING_RIGHT)
			mTexture->Render(mPosition, SDL_FLIP_NONE, 0.0);
		else
			mTexture->Render(mPosition, SDL_FLIP_HORIZONTAL, 0.0);
	}
	else 
	{
		if (mFacingDirection == FACING_RIGHT)
			mTexture->Render(mPosition, SDL_FLIP_NONE, (Uint8)255 * abs(sin(INVULN_MULTIPLIER * mInvulnTimer)), 0.0);
		else
			mTexture->Render(mPosition, SDL_FLIP_HORIZONTAL, (Uint8)255 * abs(sin(INVULN_MULTIPLIER * mInvulnTimer)), 0.0);
	}
}

void CharacterPlayable::Update(float deltaTime, SDL_Event e)
{
	Character::Update(deltaTime, e);

	int key = e.key.keysym.sym;

	//Handle any events
	switch (e.type)
	{
	case SDL_KEYDOWN:
		if (key == mInputMap[LEFT])
			mMovingLeft = true;
		else if (key == mInputMap[RIGHT])
			mMovingRight = true;
		break;

	case SDL_KEYUP:
		if (key == mInputMap[LEFT])
			mMovingLeft = false;
		else if (key == mInputMap[RIGHT])
			mMovingRight = false;
		else if (key == mInputMap[JUMP])
			Jump();
		break;
	}

	if (mInvulnerable)
	{
		mInvulnTimer -= deltaTime;
		if (mInvulnTimer <= 0.0f)
			mInvulnerable = false;
	}
}

void CharacterPlayable::OnPlayerCollision(CharacterPlayable* player)
{
	std::cout << "Colliding with other player" << std::endl;
}

void CharacterPlayable::SetState(CHARACTERSTATE newState)
{
	mState = newState;
}

CHARACTERSTATE CharacterPlayable::GetState()
{
	return mState;
}

void CharacterPlayable::KillPlayer()
{
	if (!mInvulnerable)
	{
		mLives--;
		mInvulnerable = true;
		mInvulnTimer = INVULNERABILITY_TIME;

		if (mLives <= 0)
			SetState(PLAYER_DEATH);
	}
}

void CharacterPlayable::IncrementScore(int value)
{
	mScore += value;
}

int CharacterPlayable::GetScore()
{
	return mScore;
}

void CharacterPlayable::RenderScoreAndLives(Font* font)
{
	std::string scoreString = mName + ":" + std::to_string(mScore);
	font->DrawString(scoreString, Vector2D(mScoreXPos, SCORE_HEIGHT), Vector2D(0.5f, 0.5f));
	
	std::string livesString = "Lives:" + std::to_string(mLives);
	font->DrawString(livesString, Vector2D(mScoreXPos, LIVES_HEIGHT), Vector2D(0.5f, 0.5f));
}

void CharacterPlayable::HitTile()
{
	Character::HitTile();

	if (mCurrentLevelMap->GetTileAt(mPosition.y / TILE_HEIGHT, (mPosition.x + (mSingleSpriteWidth * 0.5f)) / TILE_WIDTH) == PLATFORM)
	{
		for (int i = 0; i < mEnemiesList->size(); i++)
		{
			CharacterEnemy* currentEnemy = (*mEnemiesList)[i];
			Vector2D enemyPosition = currentEnemy->GetPosition();

			int playerTileX = (int)mPosition.x / TILE_WIDTH;
			int playerTileY = (int)mPosition.y / TILE_HEIGHT;
			int enemyTileX = (int)enemyPosition.x / TILE_WIDTH;
			int enemyTileY = (int)enemyPosition.y / TILE_HEIGHT;

			if (playerTileX == enemyTileX || playerTileX == enemyTileX + 1 || playerTileX == enemyTileX - 1)
			{
				if(enemyTileY == playerTileY - 1)
					currentEnemy->TakeDamage();
			}
		}
	}
}
