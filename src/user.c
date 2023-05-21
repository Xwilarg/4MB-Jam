#include <math.h>

#include "component/renderer.h"
#include "component/rigidbody.h"
#include "component/collider.h"
#include "audio/audio.h"
#include "user.h"

#include "config.h"

// Key info
static bool _isUpPressed, _isDownPressed, _isLeftPressed, _isRightPressed;

// Player info
static Object* _player;
static Rigidbody* _playerRb;
static bool _canJump;
static Vector _initialPos;

// Camera info
static Vector _cameraPos;
static Vector _camOffset;

static int collectibleLeft;

const Vector* GetCameraPosition(void)
{
	return &_cameraPos;
}

static void UpdateCameraPosition(void)
{
	_cameraPos = VECTOR_New(
		_camOffset.x + _player->transform->position.x,
		_camOffset.y + _player->transform->position.y,
		_camOffset.z + _player->transform->position.z
	);
}

static void ResetPlayer(void)
{
	_player->transform->position = _initialPos;
	_playerRb->linearVelocity = VECTOR_Zero(); // Reset player velocity so it doesn't go through platform
	UpdateCameraPosition();
}

static void OnPlayerCollision(Game* game, Object* collision)
{
	// If collision is NULL, it means we falled from a platform
	if (collision == NULL || collision->tag == USERTAG_TRAP)
	{
		ResetPlayer();
	}
	else if (collision->tag == USERTAG_OBJECTIVE)
	{
		GAME_RemoveObject(game, collision);
		collectibleLeft--;
		if (collectibleLeft == 0)
		{
			// TODO: Victory
		}
	}
}

void USER_Input(int key, bool isPressed)
{
	switch (key)
	{
	case 32: // Spacebar
		if (_playerRb->isOnGround)
		{
			RIGIDBODY_AddForce(_playerRb, VECTOR_New(.0f, CONFIG_JUMP_FORCE, .0f));
		}
		break;

	case 87: case 38: // W
		_isUpPressed = isPressed;
		break;

	case 83: case 40: // S
		_isDownPressed = isPressed;
		break;

	case 65: case 37: // A
		_isLeftPressed = isPressed;
		break;

	case 68: case 39: // D
		_isRightPressed = isPressed;
		break;

	case 82: // R
		ResetPlayer();
		break;
	}
}

void USER_Update(Game* g, Context* ctx)
{
	(void)g;

	// Move player
	Vector dirVector = VECTOR_Multiply(VECTOR_New(
		(_isLeftPressed ? 1.f : 0.f) + (_isRightPressed ? -1.f : 0.f),
		.0f,
		(_isUpPressed ? -1.f : 0.f) + (_isDownPressed ? 1.f : 0.f)
	), ctx->time->deltaTime * CONFIG_SPEED);
	_playerRb->linearVelocity.x = dirVector.x;
	_playerRb->linearVelocity.z = dirVector.z;

	if (dirVector.x != 0.f || dirVector.z != 0.f)
	{
		_player->transform->rotation = VECTOR_New(.0f, (float)atan2(dirVector.x, dirVector.z), .0f);
	}

	// Set camera position to follow player
	UpdateCameraPosition();
}

static Object* AddObject(Game* game, int x, int y, int z, char* mesh, char* texture)
{
	Object* obj = OBJECT_New();
	obj->transform->position = VECTOR_New(CONFIG_GRID_UNIT * x, CONFIG_GRID_UNIT_UP * y, CONFIG_GRID_UNIT * z - 200.f);

	Renderer* r = RENDERER_New(mesh, texture);
	Collider* coll = COLLIDER_New(r);

	OBJECT_AddComponent(obj, r->parent);
	OBJECT_AddComponent(obj, coll->parent);

	GAME_AddObject(game, obj);

	return obj;
}

static void AddPlatform(Game* game, int x, int y, int z)
{
	AddObject(game, x, y, z, "res/plane.mesh", "demo.tex");
}

static void AddObjective(Game* game, int x, int y, int z)
{
	Object* obj = AddObject(game, x, y, z, "demo.mesh", "demo.tex");

	obj->tag = USERTAG_OBJECTIVE;
	collectibleLeft++;
}

static void AddTrap(Game* game, int x, int y, int z)
{
	Object* obj = AddObject(game, x, y, z, "demo.mesh", "demo.tex");

	obj->tag = USERTAG_TRAP;
}

void USER_Init(Game* g, Context* ctx)
{
	(void)ctx;

	_cameraPos = VECTOR_New(0.f, 400.f, 200.f);
	_isLeftPressed = false;
	_isRightPressed = false;
	_isUpPressed = false;
	_isDownPressed = false;
	_canJump = true;
	collectibleLeft = 0;

	AUDIO_Play(AUDIO_Load("res/BGM.sound", true));

	{
		_player = OBJECT_New();
		_player->transform->position.z = -200.f; // TODO: Probably can do so we don't hardcode that uh
		_player->transform->position.y = CONFIG_GRID_UNIT_UP * 2.f;

		Renderer* r = RENDERER_New("demo.mesh", "demo.tex");
		Collider* coll = COLLIDER_New(r);
		_playerRb = RIGIDBODY_New();

		coll->onCollision = OnPlayerCollision;

		OBJECT_AddComponent(_player, r->parent);
		OBJECT_AddComponent(_player, _playerRb->parent);
		OBJECT_AddComponent(_player, coll->parent);

		GAME_AddObject(g, _player);
	}
	AddPlatform(g, 0, 1, 0);

	AddPlatform(g, 1, 1, 0);
	AddObjective(g, 1, 2, 0);

	AddPlatform(g, -1, 1, 0);
	AddPlatform(g, -1, 1, 1);
	AddPlatform(g, -2, 1, 1);
	AddPlatform(g, -3, 1, 1);
	AddPlatform(g, -3, 1, 0);
	AddTrap(g, -2, 1, 0);
	AddObjective(g, -3, 1, 0);

	_initialPos = _player->transform->position;
	_camOffset = VECTOR_New(
		_cameraPos.x - _player->transform->position.x,
		_cameraPos.y - _player->transform->position.y,
		_cameraPos.z - _player->transform->position.z
	);
}