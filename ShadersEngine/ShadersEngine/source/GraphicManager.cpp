#include "../headers/GraphicManager.h"


CGraphicManager::CGraphicManager(void)
{
	m_pDirect3DManager = 0;
	m_pCamera = 0;
}

CGraphicManager::CGraphicManager(const CGraphicManager & in_Other)
{
}
CGraphicManager::~CGraphicManager(void)
{

}

bool CGraphicManager::Initialize(int in_nWidth, int in_nHeight, HWND in_WinHandler)
{
	bool bInitializeSucess;
	/* Create Direct3DManger object. */
	m_pDirect3DManager = new CD3DManager();
	if (!m_pDirect3DManager)
		return false;
	/* Initialize Direct3DManger object */
		bInitializeSucess = m_pDirect3DManager->Initialize (in_nWidth,in_nHeight, in_WinHandler, b_vSyncEnabled, b_FullScreen, cfScreenNear, cfScreenDepth);
	if(!bInitializeSucess)
	{
		MessageBox(in_WinHandler,"Could not initialize Direct3D manager", "Error", MB_OK);
		return false;
		
	}

	/*Init Camera*/
	m_pCamera = new Camera();
	if (!m_pCamera)
	{
		MessageBox(in_WinHandler, "Could not create camera", "Error", MB_OK);
		return false;
	}

	//Set camera init position
	m_pCamera->SetPosition(0.0f, 0.0f, -0.5f);

	return true;
}

bool CGraphicManager::Frame(HWND in_WinHandler)
{
	bool bIsD3DRenderSuccesful;

	bIsD3DRenderSuccesful = Render(in_WinHandler);
	if(!bIsD3DRenderSuccesful)
	{
		return false;
	}
	return true;
}

void CGraphicManager::Exit()
{
	/* Release Direct3DManager object resources.*/
	if(m_pDirect3DManager)
	{
		m_pDirect3DManager->Exit ();
		delete m_pDirect3DManager;
		m_pDirect3DManager = 0;

		delete m_pCamera;
		m_pCamera = 0;
	
	}
	return;
}

bool CGraphicManager::Render(HWND in_WinHandler)
{
	bool result;

	/*get proper view matrix*/
	m_pCamera->Render();

	// Use the Direct3D object to render the scene.
	result = m_pDirect3DManager->Render(in_WinHandler, m_pCamera->GetViewMatrix());
	if (!result)
	{
		return false;
	}

	return true;
}
