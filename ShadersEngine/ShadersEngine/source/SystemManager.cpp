#include "../headers/SystemManager.h"


SystemManager::SystemManager(void)
{
	//Initialize our pointers to 0.
	m_pInput = 0;
	m_pGraphicMng = 0;
}

SystemManager::SystemManager(const SystemManager& in_NewSysMng)
{
}


SystemManager::~SystemManager(void)
{
}


bool SystemManager::Initialize()
{
	/* Initialize ScreenSize to 0. */
	int nScreenWidth = 0,
		nScreenHeight = 0;
	bool bIsGraphicManagerInit;
	/*Call InitWin to create window with specified resolution.*/
	InitWin(nScreenWidth, nScreenHeight);
	/*Create in put object, this will read the keyboard input.*/
	m_pInput = new CInput;
	if (!m_pInput)
	{
		return false;
	}

	/* Initializes input object.*/
	m_pInput->Initialize();

	/*Create graphics objects, this will manage and handle the rendering te graphics in the app.*/
	m_pGraphicMng = new CGraphicManager;
	if (!m_pGraphicMng)
	{
		return false;
	}

	/* Initialize Graphic object, with resolution and the window handler*/
	bIsGraphicManagerInit = m_pGraphicMng->Initialize(nScreenWidth, nScreenHeight, m_WinHandler);

	if (!bIsGraphicManagerInit)
	{
		return false;
	}

	return true;
}

void SystemManager::Exit()
{
	/*Release graphic object.*/
	if (m_pGraphicMng)
	{
		m_pGraphicMng->Exit();
		delete m_pGraphicMng;
		m_pGraphicMng = 0;
	}
	/*Release input object*/
	if (m_pInput)
	{
		delete m_pInput;
		m_pInput = 0;
	}

	/*Closes and shutdown window*/
	ShutDownWin();

	return;
}


void SystemManager::Run()
{
	MSG Message; /*< Contains message information from a thread's message queue. Source: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmsg */
	bool bIsDone = 0, bIsFrameFinish = 0; /*< Boleans to break the loop given certain rules */

	/*Initialize message structure, 1: It receives a pointer to a MSG structure. 2: and the size of the MSG struct*/
	ZeroMemory(&Message, sizeof(MSG));

	/*Loop until user quits window or until there is a exit message*/
	while (!bIsDone)
	{
		/*Handle win Messages, PeekMessage: Dispatches incoming sent messages, checks the thread message queue for a posted message, and retrieves the message (if any exist).
			1: It receives a pointer to a MSG structure.
			2: HWND A handle to the window whose messages are to be retrieved, If hWnd is NULL, PeekMessage retrieves messages for any window that belongs to the current thread.
			3: UINT The value of the first message in the range of messages to be examined.
			4: UINT The value of the last message in the range of messages to be examined.
			  If wMsgFilterMin and wMsgFilterMax are both zero, PeekMessage returns all available messages (that is, no range filtering is performed).
			5:UINT Specifies how messages are to be handled. This parameter can be one or more of the following values.
				PM_NOREMOVE: Messages are not removed from the queue after processing by PeekMessage.
				PM_REMOVE: Messages are removed from the queue after processing by PeekMessage.
				PM_NOYIELD: Prevents the system from releasing any thread that is waiting for the caller to go idle.
				https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-peekmessagea
		*/
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			/*Translates virtual-key messages into character messages.
				1: It receives a pointer to a MSG structure.
				https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-translatemessage
			*/
			TranslateMessage(&Message);
			/*Dispatches a message to a window procedure..
				1: It receives a pointer to a MSG structure.
				https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-dispatchmessage
			*/
			DispatchMessage(&Message);
		}
		/*If we receive a exit or close messages, then exit*/
		if (Message.message == WM_QUIT)
		{
			bIsDone = true;
		}
		else
		{
			/*otherwise run the frame processing*/
			bIsFrameFinish = Frame();
			if (!bIsFrameFinish)
			{
				bIsDone = true;
			}
		}
	}
	return;
}


bool SystemManager::Frame()
{
	bool bIsGraphicFinished;

	/*Check if user pressed escape and wants to exit app*/
	if (m_pInput->isKeyDown(VK_ESCAPE))
	{
		return false;
	}

	/* Start frame processing for graphics*/
	bIsGraphicFinished = m_pGraphicMng->Frame(m_WinHandler);
	if (!bIsGraphicFinished)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK SystemManager::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	/*Message received.*/
	switch (umsg)
	{
		/*Check if the user has pressed on the keyboard.*/
	case WM_KEYDOWN:
	{
		/*If user pressed something, send information to our input object to save that state.*/
		m_pInput->KeyDown((unsigned int)wparam);
		return 0;
	}
	/*Check if the user has release a key in the keyboard.*/
	case WM_KEYUP:
	{
		/*If user release something, send information to our input object to save that state.*/
		m_pInput->KeyUp((unsigned int)wparam);
	}
	/*Any other message leave it to the default message handler.*/
	default:
	{
		/*Calls the default window procedure to provide default processing for any window messages that an application does not process.
			This function ensures that every message is processed.
			1: HWND handler to the current window or thread.
			2: UINT	Message to be sent.
			3: WPARAM Addtional information about the message.
			4: LPARAM additional informatio about the message.
			https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-defwindowproca
		*/
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

void SystemManager::InitWin(int& in_Width, int& in_Height)
{
	/*WNDCLASSEX contains windows class attributes used in the RegisterClassEx https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-tagwndclassexa
		Parameters:
			1. UINT cbSize The size, in bytes, of this structure. Set this member to sizeof(WNDCLASSEX). Be sure to set this member before calling the GetClassInfoEx function.
			2. UINT style The class style(s) Defines how to update the window after moving or resizing it, how to process double-clicks of the mouse, how to allocate space for the device context, and other aspects of the window.
				for more styles: https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
				about window classes: https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-classes
			3. WNDPROC lpfnWndProc a pointer to the window procedure. You must use the CallWindowProc function to call the window procedure. https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633573(v=vs.85)
			4. int cbClsExtra The number of extra bytes to allocate following the window-class structure. The system initializes the bytes to zero.
			5. int cbWndExtra The number of extra bytes to allocate following the window instance. The system initializes the bytes to zero. If an application uses WNDCLASSEX to register a dialog box created by using the CLASS directive in the resource file, it must set this member to DLGWINDOWEXTRA.
			6. HINSTANCE hInstance A handle to the instance that contains the window procedure for the class.
			7. HICON hIcon A handle to the class icon. This member must be a handle to an icon resource. If this member is NULL, the system provides a default icon.
			8. HCURSOR hCursor A handle to the class cursor. This member must be a handle to a cursor resource. If this member is NULL, an application must explicitly set the cursor shape whenever the mouse moves into the application's window.
			9. HBRUSH hbrBackground A handle to the class background brush. This member can be a handle to the brush to be used for painting the background, or it can be a color value.
				A color value must be one of the following standard system colors (the value 1 must be added to the chosen color). If a color value is given, you must convert it to one of the following HBRUSH types:

				*COLOR_ACTIVEBORDER
				*COLOR_ACTIVECAPTION
				*COLOR_APPWORKSPACE
				*COLOR_BACKGROUND
				*COLOR_BTNFACE
				*COLOR_BTNSHADOW
				*COLOR_BTNTEXT
				*COLOR_CAPTIONTEXT
				*COLOR_GRAYTEXT
				*COLOR_HIGHLIGHT
				*COLOR_HIGHLIGHTTEXT
				*COLOR_INACTIVEBORDER
				*COLOR_INACTIVECAPTION
				*COLOR_MENU
				*COLOR_MENUTEXT
				*COLOR_SCROLLBAR
				*COLOR_WINDOW
				*COLOR_WINDOWFRAME
				*COLOR_WINDOWTEXT

				When this member is NULL, an application must paint its own background whenever it is requested to paint in its client area. To determine whether the background must be painted,
				an application can either process the WM_ERASEBKGND message or test the fErase member of the PAINTSTRUCT structure filled by the BeginPaint function.
		   10. LPCTSTR lpszMenuName Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file. If you use an integer to identify the menu, use the MAKEINTRESOURCE macro.
				  If this member is NULL, windows belonging to this class have no default menu.
		   11. LPCTSTR lpszClassName  A pointer to a null-terminated string or is an atom. If this parameter is an atom, it must be a class atom created by a previous call to the RegisterClass or RegisterClassEx function. The atom must be in the low-order word of lpszClassName; the high-order word must be zero.
				If lpszClassName is a string, it specifies the window class name. The class name can be any name registered with RegisterClass or RegisterClassEx, or any of the predefined control-class names. The maximum length for lpszClassName is 256. If lpszClassName is greater than the maximum length,
				the RegisterClassEx function will fail.
		   12. HICON hIconSm A handle to a small icon that is associated with the window class. If this member is NULL, the system searches the icon resource specified by the hIcon member for an icon of the appropriate size to use as the small icon.
	*/
	WNDCLASSEX  WndClass;

	/*Data structure contains information about the initialization and environment of a printer or a display device.
		https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-_devicemodea
	*/
	DEVMODE ScreenSettings;

	/* Position of the window in the screen.*/
	int n_PosY = 0, n_PosX = 0;

	/* Get external pointer to this object */
	s_pApplicationHandler = this;

	/* Get instance of this aplication
		If NULL is provided it gets  the file used to create the calling process (.exe file).
		https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
	*/
	m_hInstance = GetModuleHandle(NULL);

	/*Set app name*/
	m_pAppName = L"Shaders";

	/*Set windows class values */
	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	/*https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles */
	WndClass.lpfnWndProc = WndProc;								/*Send pointer to our WndProc*/
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = m_hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);			/*https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadicona */
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);			/*https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadcursora */
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  /*Gets a handle of the stock pens, brushes, etc. https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getstockobject */
	WndClass.lpszMenuName = (LPCSTR)m_pAppName;
	WndClass.lpszClassName = (LPCSTR)m_pAppName;
	WndClass.hIconSm = WndClass.hIcon;

	/*Registers a window class for subsequent use
		https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexa
	*/
	RegisterClassEx(&WndClass);
	/* Get window width and height  https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics */
	in_Width = GetSystemMetrics(SM_CXSCREEN);
	in_Height = GetSystemMetrics(SM_CYSCREEN);

	/*If it is fullscreen*/
	if (b_FullScreen)
	{
		/*Initialize ScreenSettings set memory*/
		memset(&ScreenSettings, 0, sizeof(ScreenSettings));
		ScreenSettings.dmSize = sizeof(ScreenSettings);			/*Specifies the size, in bytes, of the DEVMODE structure */
		ScreenSettings.dmPelsWidth = (unsigned long)in_Width;	/*Screen width of visible device surface*/
		ScreenSettings.dmPelsHeight = (unsigned long)in_Height;	/*Screen height of visible device surface*/
		ScreenSettings.dmBitsPerPel = 32;						/*Bits per pixel, color resolution*/
		ScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT; /*Specifies whether certain members of the DEVMODE structure have been initialized*/

		/*Changes the settings of the default display device to the specified graphic mode https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-changedisplaysettingsa
			1: lpDevMode a pointer to a DEVMODE structure that describes the new graphics mode(settings).
			2: dwFlags Indicates how the graphics mode should be changed.
				0 - The graphics mode for the current screen will be changed dynamically.
				CDS_FULLSCREEN - The mode is temporary in nature.
				CDS_GLOBAL - The settings will be saved in the global settings area so that they will affect all users on the machine.

			*/
		ChangeDisplaySettings(&ScreenSettings, CDS_FULLSCREEN);
		/*Set position to the upper left corner*/
		n_PosX = n_PosY = 0;
	}
	else
	{
		/* Set Default size */
		in_Width = WINHEIGHT;
		in_Height = WINWIDTH;

		/*Center window*/
		n_PosX = (GetSystemMetrics(SM_CXSCREEN) - in_Width) / 2;
		n_PosY = (GetSystemMetrics(SM_CYSCREEN) - in_Height) / 2;
	}

	/* This function creates an overlapped, pop-up, or child window with an extended style; otherwise, this function is identical to the CreateWindow function
		1: DWORD dwExStyle, Specifies the extended style of the window.
			*WS_EX_APPWINDOW - Forces a top-level window onto the taskbar when the window is visible.
			(*more options in the link below)
		2: LPCTSTR lpClassName, window Class name Long pointer to a null-terminated string or an integer atom. If this parameter is an atom, it must be a global atom created by a previous call to the RegisterClass function.
		3: LPCTSTR lpWindowName, Long pointer to a null-terminated string that specifies the window name.
		4: DWORD dwStyle, Specifies the style of the window being created.
			*WS_DISABLED - Creates a window that is initially disabled. A disabled window cannot receive input from the user.
			*WS_CLIPSIBLINGS - Clips child windows relative to each other; that is, when a particular child window receives a WM_PAINT message,
								the WS_CLIPSIBLINGS style clips all other overlapping child windows out of the region of the child window to be updated.
			*WS_CLIPCHILDREN - Excludes the area occupied by child windows when drawing occurs within the parent window. This style is used when creating the parent window.
			*WS_POPUP	- Creates a pop-up window.
		5: int x, Specifies the initial horizontal position of the window.
		6: int y, Specifies the initial vertical position of the window.
		7: int nWidth, Specifies the width, in device units, of the window.
		8: int nHeight, Specifies the height, in device units, of the window.
		9: HWND hWndParent, Handle to the parent or owner window of the window being created.
		10: HMENU hMenu, Handle to a menu, or specifies a child-window identifier, depending on the window style.
		11: HINSTANCE hInstance, Handle to the instance of the module to be associated with the window.
		12: LPVOID lpParam, Long pointer to a value to be passed to the window through the CREATESTRUCT structure passed in the lParam parameter the WM_CREATE message.
		(*This is our parent/only window)
	https://docs.microsoft.com/en-us/previous-versions/ms960010(v%3Dmsdn.10)
	*/
	m_WinHandler = CreateWindowEx(WS_EX_APPWINDOW, (LPCSTR)m_pAppName, (LPCSTR)m_pAppName, WS_DISABLED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		n_PosX, n_PosY, in_Width, in_Height, NULL, NULL, m_hInstance, NULL);

	/*Sets the specified window's show state.
		1: HWND hWnd, A handle to the window.
		2: int  nCmdShow, Controls how the window is to be shown
			*SW_SHOW - Activates the window and displays it in its current size and position.
	https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
	*/
	ShowWindow(m_WinHandler, SW_SHOW);
	/*Brings the thread that created the specified window into the foreground and activates the window. Keyboard input is directed to the window, and various visual cues are changed for the user.
	The system assigns a slightly higher priority to the thread that created the foreground window than it does to other threads.
		1:  HWND hWnd, A handle to the window that should be activated and brought to the foreground.
	*/
	SetForegroundWindow(m_WinHandler);
	/*Sets the keyboard focus to the specified window. The window must be attached to the calling thread's message queue.
		1: HWND hWnd, A handle to the window that will receive the keyboard input. If this parameter is NULL, keystrokes are ignored.
	*/
	SetFocus(m_WinHandler);
	/*Displays or hides the cursor.
		1: BOOL bShow, If bShow is TRUE, the display count is incremented by one. If bShow is FALSE, the display count is decremented by one.
	*/
	ShowCursor(true);
	return;
}

void SystemManager::ShutDownWin()
{
	/*Displays or hides the cursor.
		1: BOOL bShow, If bShow is TRUE, the display count is incremented by one. If bShow is FALSE, the display count is decremented by one.
	*/
	ShowCursor(true);

	/* Remove FullScreen settings */
	if (b_FullScreen)
	{
		/*Changes the settings of the default display device to the specified graphic mode https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-changedisplaysettingsa
				1: lpDevMode a pointer to a DEVMODE structure that describes the new graphics mode(settings).
				2: dwFlags Indicates how the graphics mode should be changed.
				0 - The graphics mode for the current screen will be changed dynamically.
				CDS_FULLSCREEN - The mode is temporary in nature.
				CDS_GLOBAL - The settings will be saved in the global settings area so that they will affect all users on the machine.
		*/
		ChangeDisplaySettings(NULL, 0);
	}

	/*Destroys the specified window. The function sends WM_DESTROY and WM_NCDESTROY messages to the window to deactivate it and remove the keyboard focus from it.
		1: HWND hWnd, A handle to the window to be destroyed.
	*/
	DestroyWindow(m_WinHandler);
	/*Set window handler to null*/
	m_WinHandler = NULL;

	/*Unregisters a window class, freeing the memory required for the class.
		1: LPCSTR    lpClassName, A null-terminated string or a class atom. If lpClassName is a string, it specifies the window class name.
		2: HINSTANCE hInstance, A handle to the instance of the module that created the class.
		*/
	UnregisterClass((LPCSTR)m_pAppName, m_hInstance);
	/*Set instance to null*/
	m_hInstance = NULL;
	/*Set our object class handler to null*/
	s_pApplicationHandler = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		/* Check if the window is being destroyed*/
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	/* Check if the window is being closed*/
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	/* All other messages pass to the message handler in the systemmanager */
	default:
	{
		return s_pApplicationHandler->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}