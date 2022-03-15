#ifndef _SYSTEMMANAGER_H_
#define _SYSTEMMANAGER_H_
//just include one time this library, avoiding duplicate librearies.

#define WIN32_LEAN_AND_MEAN //Exclude some of the less used APIs, this helps to speed up the build processs.

//#include <Windows.h> //Windows library so we can use de create and destroy windows and other useful win32 functions.

//WINDOWS DATA TYPES https://docs.microsoft.com/en-us/windows/desktop/winprog/windows-data-types
#include "InputManager.h"
#include "GraphicManager.h"
constexpr auto WINHEIGHT = 1500;
constexpr auto WINWIDTH = 1000;
/*! \class SystemManager
	\class for manage the window cycle messages as well as the graphic and input entries.
*/
class SystemManager
{
public:
	SystemManager(void);
	SystemManager(const SystemManager& in_NewSysMng);
	~SystemManager(void);

	/**
		SystemManager::Initialize function
			Description: Set up for the application, It first calls	InitializeWindows, it also creates and initializes both the input and graphics objects
			that the application will use for handling user input and rendering graphics to the screen.
	**/
	bool Initialize();

	/**
		SystemManager::Run function
			Description: Here the application will loop and do all the processes we need until we quit.
	**/
	void Run();

	/**
		SystemManager::Exit function
			Description: Does the clean up, it shuts down and releases everything associated with the graphics and input objects, closes the window and any handler linked to it.
	**/
	void Exit();
	/**
		SystemManager::MessageHandler function
			Description: We get direct windows system message, we can obtain information from keys pressed that we will pass to the input.
			1: HWND handler to the current window or thread.
			2: UINT	Message to be sent.
			3: WPARAM Addtional information about the message.
			4: LPARAM additional informatio about the message.
	**/
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	/**
		SystemManager::Frame function
			Description: here the application processing is going to be processed, input process and graphics stuff, it is called in the Run function.
	**/
	bool Frame();
	/**
		SystemManager::InitWin function
			Description: Will create the window used for render. It returns in_Width and in_Height back to the calling function to be used along the whole application.
	**/
	void InitWin(int& in_Width, int& in_Height);
	/**
		SystemManager::ShutDownWin function
			Description: Returns screen settings back to normal, and releases memory associated to window, instance, and application handler.
	**/
	void ShutDownWin();

private:
	LPCWSTR m_pAppName;				/*!< A pointer to a constant null temrinated string, name of the app. */
	HINSTANCE m_hInstance;			/*!< Handle of a instance, application instance.*/
	HWND m_WinHandler;				/*!< A handle to a window.*/

	CInput *m_pInput;				/*!< pointer that handles the input manager*/
	CGraphicManager *m_pGraphicMng; /*!< pointer to handle the graphic manager*/

};

/**
	WndProcfunction
		Description: Function where a window sends its messages to.
		1: HWND handler to the current window or thread.
		2: UINT	Message to be sent.
		3: WPARAM Addtional information about the message.
		4: LPARAM additional informatio about the message.
**/
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static SystemManager* s_pApplicationHandler = 0;  /*!< Object-Class instance*/

#endif
