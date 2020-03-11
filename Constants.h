#pragma once

//Screen dimensions
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 416

//movement constants
#define MOVEMENTSPEED 60.0f
#define GRAVITY 96.0f
#define JUMP_FORCE_DECREMENT 400.0f
#define INITIAL_JUMP_FORCE 400.0f
#define INITIAL_JUMP_FORCE_SMALL 200.0f
#define KOOPA_SPEED 50.0f
#define COIN_SPEED 25.0f

//Map constants
#define MAP_HEIGHT 13
#define MAP_WIDTH 16
#define TILE_WIDTH 32
#define TILE_HEIGHT 32

#define SCREENSHAKE_DURATION 0.25f

//Enemy constants
#define INJURED_TIME 2.5f
#define ENEMY_SPAWN_TIME 10.0f
#define ANIMATION_DELAY 0.25f
#define KOOPA_FRAME_COUNT 2
#define KOOPA_MOVING_FRAME 0
#define KOOPA_DAMAGED_FRAME 1

//Coin constants
#define COIN_SPAWN_TIME 15.0f
#define COIN_SCORE 1
#define COIN_FRAME_COUNT 5