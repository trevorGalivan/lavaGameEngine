#include "inputSystem.h"

#include "engine.h"

void ButtonInputSystem::update(Engine& engine, double dt)
{
	instantaneousInputs.clear();
	while (!inputQueue.empty()) {
		buttonEvent currentInput = inputQueue.front();
		inputQueue.pop();
		
		instantaneousInputs.push_back(currentInput);
	}

	//TODO: Support alternative forms of input (mouse, scrollwheel, mouse button)
}

bool ButtonInputSystem::justPressed(Button button) {
	for (auto i = instantaneousInputs.begin(), end = instantaneousInputs.end(); i != end; ++i) {
		if (i->button == button && i->action == 1) {
			return true;
		}
	}
	return false;
}
#include <iostream>
Button ButtonInputSystem::controlToButton(unsigned int player, Control control) {
	return playerControls[player-1][control];
}

bool ButtonInputSystem::justPressed(unsigned int player, Control control) {
	Button button = controlToButton(player, control);

	for (auto i = instantaneousInputs.begin(), end = instantaneousInputs.end(); i != end; ++i) {
		if (i->button == button && i->action == 1) {
			return true;
		}
	}
	return false;
}

bool ButtonInputSystem::justReleased(Button button) {
	for (auto i = instantaneousInputs.begin(), end = instantaneousInputs.end(); i != end; ++i) {
		if (i->button == button && i->action == 0) {
			return true;
		}
	}
	return false;
}

bool ButtonInputSystem::justReleased(unsigned int player, Control control) {
	Button button = controlToButton(player, control);

	for (auto i = instantaneousInputs.begin(), end = instantaneousInputs.end(); i != end; ++i) {
		if (i->button == button && i->action == 0) {
			return true;
		}
	}
	return false;
}

void ButtonInputSystem::pushButtonAction(buttonEvent event)
{
	inputQueue.push(event);
}

bool ButtonInputSystem::isPressed(Engine& engine, unsigned int player, Control control)
{
	return engine.checkButtonState(controlToButton(player, control));
}