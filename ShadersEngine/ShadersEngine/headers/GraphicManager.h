#ifndef _GRAPHICSMANAGER_H_
#define _GRAPHICSMANAGER_H_

//#include <Windows.h>
const bool b_FullScreen = false;		/*< Variable to enable fullscreen*/
const bool b_vSyncEnabled = true;		/*< variable to enable vertical synchronyzation*/
const float cfScreenDepth = 1000.0f;	/*< Value for how far is the screen */
const float cfScreenNear = 0.1F;		/*< Value ho near is the screen */
#pragma comment(lib, "d3d12.lib")		/*<Contains all Direct#D functionality for setting up and drawing 3D grpahics in DirectX12. */
#pragma comment(lib, "dxgi.lib")		/*<Contains tools to interface with the hardware on the computer to obtain info about the refresh rate o the monitor, card being used and more.*/
#pragma comment(lib, "d3dcompiler.lib") /*<Contains tools to compile shaders. */



#include "D3DManager.h"
#include "Camera.h"



/*! \class CGraphicManager
	\class for manage all the graphics objects, settings and variables.
*/
class CGraphicManager
{
public:
	CGraphicManager(void);
	CGraphicManager(const CGraphicManager &in_Other);
	~CGraphicManager(void);

	/**
	CGraphicManager::Initialize function
		Description: Create D3DManager object and call its initialize function, send width,
					height, window handler and globals, to set up DX12 settings.
	**/
	bool Initialize(int in_nWidth, int in_nHeight, HWND in_WinHandler);
	/**
	CGraphicManager::Frame function
		Description: Calls Render everytime needed.
	**/
	bool Frame(HWND);
	/**
	CGraphicManager::Exit function
		Description: Shut down all graphics objects.
	**/
	void Exit();


private:
	bool Render(HWND);
	CD3DManager* m_pDirect3DManager; /*< Pointer to our D3DManager class object */
	Camera* m_pCamera;/*Pointer to our camera*/


};

#endif