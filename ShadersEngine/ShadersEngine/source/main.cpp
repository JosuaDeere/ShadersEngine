#include "../headers/SystemManager.h"
/********************************************************************************
*																				*
*																				*
*				WIN MAIN function												*
*			Description: Entry point function.									*
*	Params: h_instance- handle to an instance, is the base address of			*
*			the module in the memory, actual instance.							*
*			h_PrevInstance- previous instance.									*
*			p_ScmdLine- a pointer to a null-terminated string of 8-bits			*
*			Command line arguments.												*
*			iCmdshow- integer to indicate if window should be displayed.		*
*																				*
*																				*
********************************************************************************/
#include <shlobj.h>
#include <strsafe.h>
#include <string>
int WINAPI WinMain(HINSTANCE h_Instance, HINSTANCE h_PrevInstance, PSTR p_Scmdline, int iCmdshow)
{

	//Create SystemManager pointer, to handle our program.
	SystemManager  *p_mySystemManager;

	//Create a new instance.
	p_mySystemManager = new SystemManager;
	//If it creates fine, we can proceed
	if (!p_mySystemManager)
		return 0;
	//If we can initiliaze  our new instance then we can run it.		
	if (p_mySystemManager->Initialize())
	{
		//Run all the application code until it completes.
		p_mySystemManager->Run();
	}
	//ShutDown the SystemManager and clean up the object.
	p_mySystemManager->Exit();
	//Clean
	delete p_mySystemManager;
	p_mySystemManager = 0;
	return 0;
}