#pragma once
#include "system.h"
#include "componentManager.h"
#include "keyMappings.h"

#include <queue>
#include <functional>
#include <array>

class Engine;
class ButtonInputSystem;

constexpr unsigned int numPlayers = 2;

// provides API independant interface for callback-based and polling-based input

typedef struct buttonEvent {
	Button button;    // Unique button-device pair
	int action;    // 0 for release, 1 for press
	bool shift; // Mods that where held down when key was pressed
	bool ctrl;
	bool alt;
}ButtonEvent;

enum Control {
	Control_MoveLeft,
	Control_MoveRight,
	Control_Jump,
	Control_Up,
	Control_Down,
	Control_Attack,
	Control_Special,
	Control_Throw,
	CONTROL_TOTAL_SIZE
};

constexpr std::array<Button, CONTROL_TOTAL_SIZE> p1Defaults =
{
	KEY_A,
	KEY_D,
	KEY_SPACE,
	KEY_W,
	KEY_S,
	KEY_F,
	KEY_E,
	KEY_Q,
};

constexpr std::array<Button, CONTROL_TOTAL_SIZE> p2Defaults =
{
	GAMEPAD1_LEFT,
	GAMEPAD1_RIGHT,
	GAMEPAD1_A,
	GAMEPAD1_UP,
	GAMEPAD1_DOWN,
	GAMEPAD1_X,
	GAMEPAD1_Y,
	GAMEPAD1_B,
};

class ButtonInputSystem : System
{
	std::queue<buttonEvent> inputQueue;

	std::vector<buttonEvent> instantaneousInputs;

	Button controlToButton(unsigned int player, Control control);
public:
	
	std::array<std::array<Button, CONTROL_TOTAL_SIZE>, numPlayers> playerControls =
	{
		p1Defaults,
		p2Defaults
	};

	bool justPressed(Button button);

	bool justPressed(unsigned int player, Control control);

	bool justReleased(Button button);

	bool justReleased(unsigned int player, Control control);

	bool isPressed(Engine& engine, unsigned int player, Control control);

	void pushButtonAction(buttonEvent);
	//bool getButtonState(Button button);

	void update(Engine& engine, double dt);
};
