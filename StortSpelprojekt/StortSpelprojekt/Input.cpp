#include "Input.h"
#include <iostream>
Input::Input()
{
}

Input::Input(HWND window, size_t width, size_t height) : hwnd(window), height(height), width(width)
{
	mouse.SetMode(DirectX::Mouse::MODE_ABSOLUTE);
};
bool Input::GetKey(DirectX::Keyboard::Keys key) const
{
	return keyboard.GetState().IsKeyDown(key);
}

bool Input::GetKeyDown(DirectX::Keyboard::Keys key) const
{
	return keyboardButtons.IsKeyPressed(key);
}

bool Input::GetKeyUp(DirectX::Keyboard::Keys key) const
{
	return keyboardButtons.IsKeyReleased(key);
}

void Input::SetMouseMode(DirectX::Mouse::Mode mode)
{

	mouse.SetMode(mode);
}



bool Input::GetLeftMouseKey() const
{
	return  mouse.GetState().leftButton;
}

bool Input::GetLeftMouseKeyDown() const
{
	
	return mouseButtons.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
}

bool Input::GetLeftMouseKeyUp() const
{
	return mouseButtons.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
}

bool Input::GetRightMouseKey() const
{
	return mouse.GetState().rightButton;
}

bool Input::GetRightMouseKeyDown() const
{
	return mouseButtons.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
}

bool Input::GetRightMouseKeyUp() const
{
	return mouseButtons.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
}

POINT Input::GetMousePos() const
{

	return POINT{ mouse.GetState().x,mouse.GetState().y };
}

Input::~Input()
{
}

void Input::UpdateInputs()
{
	mouseButtons.Update(mouse.GetState());
	keyboardButtons.Update(keyboard.GetState());


	if (GetLeftMouseKey())
	{
		//std::cout << "testSpam";
	}
	if (GetLeftMouseKeyDown())
	{
		std::cout << "DOWN";
	}
	if (GetLeftMouseKeyUp())
	{
		std::cout << "UP";
	}
}

void Input::UpdateMsg(UINT umsg, WPARAM wParam, LPARAM lParam)
{
	DirectX::Keyboard::ProcessMessage(umsg, wParam, lParam);
	DirectX::Mouse::ProcessMessage(umsg, wParam, lParam);
}
