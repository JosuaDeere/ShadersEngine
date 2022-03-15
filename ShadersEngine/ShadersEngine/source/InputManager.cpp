#include "../headers/InputManager.h"


CInput::CInput(void)
{
}

CInput::CInput(const CInput& in_Other)
{
}

CInput::~CInput(void)
{
}

void CInput::Initialize()
{
	/*Set all keys as released and not pressed*/
	for (int i = 0; i < 256; i++)
	{
		m_Keys[i] = false;
	}
	return;
}

void CInput::KeyDown(unsigned int in_KeyValue)
{
	/*If key is pressed, save state in array*/
	m_Keys[in_KeyValue] = true;
	return;
}

void CInput::KeyUp(unsigned int in_KeyValue)
{
	/*If key is released, clear state of that key*/
	m_Keys[in_KeyValue] = false;
}

bool CInput::isKeyDown(unsigned int in_KeyValue)
{
	/*Return key being pressed.*/
	return m_Keys[in_KeyValue];
}