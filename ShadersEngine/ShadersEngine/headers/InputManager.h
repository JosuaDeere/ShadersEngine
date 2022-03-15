#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

/*! \class CInput
	\class for manage the input from the user, When queried it eill tell the calling functions if a certain key
	is pressed.
*/
class CInput
{
public:
	CInput(void);
	CInput(const CInput& in_Other);
	~CInput(void);
	/**
		CInput::Initialize function
			Description: Initialize state from all the keys.
	**/
	void Initialize();
	/**
		CInput::KeyDown function
			Description: Set state for key that was pressed
			1: unsigend int in_KeyValue, Key pressed value.
	**/
	void KeyDown(unsigned int in_KeyValue);
	/**
		CInput::KeyDown function
			Description: Set state for key that was released
			1: unsigend int in_KeyValue, Key released value.
	**/
	void KeyUp(unsigned int in_KeyValue);
	/**
		CInput::isKeyDown function
			Description: Check if a keys is being pressed
			1: unsigend int in_KeyValue, Key being pressed value.
	**/
	bool isKeyDown(unsigned int in_KeyValue);

private:
	bool m_Keys[256];	/*< array of booleans to see which keys was pressed. */
};

#endif