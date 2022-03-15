#ifndef _D3DMANAGER_H_
#define _D3DMANAGER_H_
#include "LibrariesAndHeaders.h"
#include "ShadersManager.h"



class CD3DManager
{
public:
	CD3DManager(void);
	//CD3DManager(const CD3DManager&);
	~CD3DManager(void);
	/**
	CD3DManager::Initialize function
		Description: Set up for DirectX12, Screen Height and width are window height and width,
		Direct3D use these to initialize using same window dimensions, WindowHandler is needed
		so Direct3D can access the windows previously created, Fullscreen is for run in Fullscreen
		or windowed mode, Vsync is for Direct3D render according to monitor refresh or as fast as possible.
	**/
	bool Initialize(int in_nScreenHeight, int in_nScreenWidth, HWND in_hWindow, bool in_bVSync, bool in_bFullScreen, float in_fNear, float in_fDepth);
	/**
	CD3DManager::Exit function
		Description: Clean and release all pointers used.
	**/
	void Exit();
	/*Render de scene*/
	bool Render(HWND in_hWindow, XMMATRIX in_ViewMatrix);
	/**
	CD3DManager::Frame function
		Description:  Update frame-based values.
	**/
	bool Frame();

	
	/* functions to get the matrix*/
	XMMATRIX GetProjectionMatrix();
	XMMATRIX GetWorldMatrix();
	XMMATRIX GetOrthoMatrix();
	void GetProjectionMatrix(XMMATRIX& io_projectionMatrix);
	void GetWorldMatrix(XMMATRIX& io_worldMatrix);
	void GetOrthoMatrix(XMMATRIX& io_orthoMatrix);
	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetGraphicsCommandList();
	float GetAspectRatio();
	ID3D12CommandAllocator* getCommandAllocator();
	ID3D12PipelineState* getPipelineStateObject();
	ID3D12RootSignature* getRootSignature();

private:
	/*******************************************************vARIABLES*****************************************************/
	float m_fAspectRatio;		/*Aspect ratio*/
	float m_fFieldOfView;		/*Field of view, what my camera can see*/
	bool  m_bIsVSyncEnabled;	/*vertical synchronization */
	int   m_nVideoCardMemory;	/*Video card memory*/
	unsigned long long m_ullFenceValue;	/*< Fence value */
	HANDLE		m_hFenceEvent;			/*< Handle to our Fence Event*/
	XMMATRIX	m_projectionMatrix;		/*projection matrix*/
	XMMATRIX	m_worldMatrix;			/*This matrix is used to convert the vertices of our objects into vertices in the 3D scene*/
	XMMATRIX	m_orthoMatrix;			/*This matrix is used for rendering 2D elements like user interfaces on the screen allowing us to skip the 3D rendering*/
	ID3DBlob*	m_pVertexShader;		/*Pointer to my vertex shader*/
	ID3DBlob*	m_pPixelShader;			/*pointer to my pixel shader*/
	/*Find Definitions */
	UINT			m_uiRenderTargetViewDescriptorSize;
	UINT			m_uiFrameIndex;							 /*<Fence index for syncrhonization*/
	UINT8*			m_pVertexData;
	D3D12_VIEWPORT	m_Viewport;
	D3D12_RECT		m_scissorRect;
	ID3D12RootSignature* m_pRootSignature;
	ID3D12Resource*		 m_pShaderBuffer[BufferCount];				/*Resource to help us map our vertex data*/
	char				 m_cVideoCardDescription[VideoCardDescSize];/*< VideoCard information*/
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;					/*Object to use our vertex data*/
	D3D12_INPUT_ELEMENT_DESC* m_pInputLayout;						/*Input layout, holds our vertex and pixel data structure*/
	CShadersManager*		 m_pShaderManager;						/*pointer to our shader manager object which will handle all shader resourcers and initializations including constant buffers.*/
	D3D12_RASTERIZER_DESC	 m_rasterizerDesc;
	D3D12_BLEND_DESC		 m_blendDesc;
	D3D12_DEPTH_STENCIL_DESC m_depthStencilDesc;
	/*<ID3D12Device Represents a virtual adapter; it is used to create command allocators, command lists,
		command queues, fences, resources, pipeline state objects, heaps, root signatures,
		samplers, and many resource views.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12device#methods
	*/
	ID3D12Device* m_pDevice;

	/*<ID3D12CommandQueue Provides methods for submitting command lists, synchronizing command list execution,
		instrumenting the command queue, and updating resource tile mappings.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12commandqueue#methods
	*/
	ID3D12CommandQueue* m_pCommandQueue;

	/*< Interface that implements one or more surfaces for storing rendered data before presenting it to an output.
		SwapChain2 provide methods to support swap back buffer scaling and lower-latency swap chains.
		SwapChain 3 extends previous Intances by giving support getting the index of the swap chain's current back buffer and support for color space.
		Mehods:
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_4/nn-dxgi1_4-idxgiswapchain3#methods
	*/
	IDXGISwapChain3* m_pSwapChain;

	/*< Collection of contiguous allocations of descriptors, one allocation for every descriptor.
		Descriptor heaps contain many object types that are not part of a Pipeline State Object (PSO),
		such as Shader Resource Views (SRVs), Unordered Access Views (UAVs), Constant Buffer Views (CBVs),
		and Samplers.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12descriptorheap#methods
	*/
	ID3D12DescriptorHeap* m_pRenderTargetViewHeap;

	/*<Encapsulates a generalized ability of the CPU and GPU to read and write to physical memory, or heaps.
		It contains abstractions for organizing and manipulating simple arrays of data as well as
		multidimensional data optimized for shader sampling.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12resource#methods
	*/
	ID3D12Resource* m_pBackBufferRenderTarget[BufferCount];

	/*< Represents the allocations of storage for graphics processing unit (GPU) commands.
		ID3D12CommandAllocator::Reset : Indicates to re-use the memory that is associated with the command allocator.
	*/
	ID3D12CommandAllocator* m_pCommandAllocator;

	/*<Encapsulates a list of graphics commands for rendering. Includes APIs for instrumenting the command list execution,
		and for setting and clearing the pipeline state.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12graphicscommandlist#methods
	*/
	ID3D12GraphicsCommandList* m_pGraphicsCommandList;

	/*<Represents the state of all currently set shaders as well as certain fixed function state objects.
		ID3D12PipelineState::GetCachedBlob : Gets the cached blob representing the pipeline state.
	*/
	ID3D12PipelineState* m_pPipelineState;
	ID3D12Resource* m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	/*<Represents a fence, an object used for synchronization of the CPU and one or more GPUs.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12fence#methods
	*/
	ID3D12Fence* m_pFence;

	/*******************************************************FUNCTIONS*****************************************************/
	/*
	* Function to Load the pipeline, basically the initialization of DirectX, and the matrixes
	*/
	bool LoadPipeline(HWND in_hWindow, int in_nScreenHeight, int in_nScreenWidth, bool in_bVSync, bool in_bFullScreen);
	bool LoadAssets(HWND in_hWindow, WCHAR* in_pszVertexShader, WCHAR* in_pszPixelShader = nullptr);
	bool PopulateCommandList(HWND in_hWindow);
	bool WaitForPreviousFrame(HWND in_hWindow);
	bool CreateRootDescriptor(HWND in_hWindow);
	void CreateRasterizer();
	void CreateBlend();
	void CreateDepthStencil();
	bool CreatePSO(HWND in_hWindow);
	bool CreateVertexBufferView(HWND in_hWindow);
	bool CreateFence(HWND in_hWindow);

	/*Variables and functions that needs to be into a model class */
private:
	bool CreateIndexBuffer(HWND in_hWindow);
	UINT8* puiIndexBufferData;
	ID3D12Resource* m_pIndexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure holding information about the index buffer
	
};
/**** GUID (a identifier) GUIDs identify objects such as interfaces, manager entry-point vectors (EPVs), and class objects.
A GUID is a 128-bit value consisting of one group of 8 hexadecimal digits,
followed by three groups of 4 hexadecimal digits each, followed by one group of 12 hexadecimal digits.
The following example GUID shows the groupings of hexadecimal digits in a GUID: 6B29FC40-CA47-1067-B31D-00DD010662DA
***/
#endif