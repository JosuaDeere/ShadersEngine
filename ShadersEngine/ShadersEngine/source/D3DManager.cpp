#include "../headers/D3DManager.h"

/**********************************PUBLIC FUNCTIONS***************************************/
CD3DManager::CD3DManager(void)
{
	/*<Initialize all the members*/
	m_pDevice = 0;
	m_pCommandQueue = 0;
	m_pSwapChain = 0;
	m_pRenderTargetViewHeap = 0;
	m_pBackBufferRenderTarget[0] = 0;
	m_pBackBufferRenderTarget[1] = 0;
	m_pCommandAllocator = 0;
	m_pGraphicsCommandList = 0;
	m_pPipelineState = 0;
	m_pFence = 0;
	m_hFenceEvent = 0;
	m_ullFenceValue = 0;
	m_uiFrameIndex = 0;
	m_uiRenderTargetViewDescriptorSize=0;
	m_fAspectRatio = 0.0f;
	m_fFieldOfView = 0.0f;
	m_pShaderManager = new CShadersManager();
}


CD3DManager::~CD3DManager(void)
{
}

bool CD3DManager::Initialize(int in_nScreenHeight, int in_nScreenWidth, HWND in_hWindow, bool in_bVSync, bool in_bFullScreen, float in_fNear, float in_fDepth)
{
	m_fAspectRatio =(float) in_nScreenWidth / in_nScreenHeight;
	/*init viewport and m_scissorRect*/
	m_Viewport.Height	= static_cast<float>(in_nScreenHeight);
	m_Viewport.Width	= static_cast<float>(in_nScreenWidth);
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_scissorRect.left	= 0;
	m_scissorRect.top	= 0;
	m_scissorRect.right = static_cast<LONG>(in_nScreenWidth);
	m_scissorRect.bottom = static_cast<LONG>(in_nScreenHeight);

	const wchar_t* szpShaderPath = L"assets/FX/TexturteShader.hlsl";
	LoadPipeline(in_hWindow, in_nScreenHeight, in_nScreenWidth, in_bVSync, in_bFullScreen);
	LoadAssets(in_hWindow, (WCHAR*)szpShaderPath);

	// create projection matrix
	m_fFieldOfView = PI / 4.0f;

	/*Builds a left-handed perspective projection matrix based on a field of view.*/
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fFieldOfView, m_fAspectRatio, in_fNear, in_fDepth);

	/*Set it up as identity, it will change after*/
	m_worldMatrix = XMMatrixIdentity();

	/*Create Ortho matrix*/
	m_orthoMatrix = XMMatrixOrthographicLH((float)in_nScreenWidth, (float)in_nScreenHeight, in_fNear, in_fDepth);

	return true;
}

void CD3DManager::Exit()
{
	int nError;

	
	/*Before shutting down set to windowed mode */
	if (m_pSwapChain)
	{
		m_pSwapChain->SetFullscreenState(false, NULL);
	}

	/* Close the object handle to the fence event.*/
	nError = CloseHandle(m_hFenceEvent);
	if (nError == 0)
	{
	}

	/* Release the fence. */
	if (m_pFence)
	{
		m_pFence->Release();
		m_pFence = 0;
	}

	/* Release the empty pipe line state. */
	if (m_pPipelineState)
	{
		m_pPipelineState->Release();
		m_pPipelineState = 0;
	}

	/* Release the command list.*/
	if (m_pGraphicsCommandList)
	{
		m_pGraphicsCommandList->Release();
		m_pGraphicsCommandList = 0;
	}

	/* Release the command allocator.*/
	if (m_pCommandAllocator)
	{
		m_pCommandAllocator->Release();
		m_pCommandAllocator = 0;
	}

	/* Release the back buffer render target views.*/
	if (m_pBackBufferRenderTarget[0])
	{
		m_pBackBufferRenderTarget[0]->Release();
		m_pBackBufferRenderTarget[0] = 0;
	}
	if (m_pBackBufferRenderTarget[1])
	{
		m_pBackBufferRenderTarget[1]->Release();
		m_pBackBufferRenderTarget[1] = 0;
	}

	/*Release the render target view heap.*/
	if (m_pRenderTargetViewHeap)
	{
		m_pRenderTargetViewHeap->Release();
		m_pRenderTargetViewHeap = 0;
	}

	/* Release the swap chain. */
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = 0;
	}

	/* Release the command queue.*/
	if (m_pCommandQueue)
	{
		m_pCommandQueue->Release();
		m_pCommandQueue = 0;
	}

	/* Release the device. */
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = 0;
	}
	/*Release Root signature*/
	if (m_pRootSignature)
	{
		m_pRootSignature->Release();
		m_pRootSignature = 0;
	}
	/*Release Vertex Buffer*/
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = 0;
	}
	if (m_pShaderManager)
	{
		delete m_pShaderManager;
		m_pShaderManager = nullptr;
	}
	return;
}

bool CD3DManager::Frame()
{
	return true;
}

bool CD3DManager::Render(HWND in_hWindow,XMMATRIX in_ViewMatrix)
{

	HRESULT isSuccess;
	/*Update my matrixes*/
	m_pShaderManager->SetShaderConstantBuffer(m_worldMatrix, in_ViewMatrix, m_projectionMatrix);
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList(in_hWindow);

	/* represents an ordered set of commands that the GPU executes,
	while allowing for extension to support other command lists than just those for graphics*/
	ID3D12CommandList* ppCommandLists[] = { m_pGraphicsCommandList };
	/* Execute the list of commands.
		https://docs.microsoft.com/es-es/windows/win32/api/d3d12/nf-d3d12-id3d12commandqueue-executecommandlists
	*/
	
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/*present the back buffer to the screen*/
	if (m_bIsVSyncEnabled)
	{
		/*Lock to screen refresh rate
			Presents a rendered image to the user.
			UINT SyncInterval, An integer that specifies how to synchronize presentation of a frame with the vertical blank.
			UINT Flags An integer value that contains swap-chain presentation options.
			https://docs.microsoft.com/es-es/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present
		*/
		// Present the frame.
		isSuccess = m_pSwapChain->Present(1, 0);
		if (FAILED(isSuccess))
		{
			MessageBox(in_hWindow, "Present failed.", "DirectX Present failed", MB_OK);
			return false;
		}
	}
	else
	{
		/* Present as fast as possible.*/
		// Present the frame.
		isSuccess = m_pSwapChain->Present(0, 0);
		if (FAILED(isSuccess))
		{
			MessageBox(in_hWindow, "Present failed.", "DirectX Present failed", MB_OK);
			return false;
		}
	}
	

	WaitForPreviousFrame(in_hWindow);
	return true;

}

/**********************************GETTER FUNCTIONS****************************************/
XMMATRIX CD3DManager::GetProjectionMatrix()
{
	return m_projectionMatrix;
}
XMMATRIX CD3DManager::GetWorldMatrix()
{
	return m_worldMatrix;
}
XMMATRIX CD3DManager::GetOrthoMatrix()
{
	return m_orthoMatrix;
}
void	 CD3DManager::GetProjectionMatrix(XMMATRIX& io_projectionMatrix)
{
	io_projectionMatrix = m_projectionMatrix;
}
void	 CD3DManager::GetWorldMatrix(XMMATRIX& io_worldMatrix)
{
	io_worldMatrix = m_worldMatrix;
}
void	 CD3DManager::GetOrthoMatrix(XMMATRIX& io_orthoMatrix)
{
	io_orthoMatrix = m_orthoMatrix;
}
ID3D12Device* CD3DManager::GetDevice()
{
	return m_pDevice;
}
ID3D12GraphicsCommandList* CD3DManager::GetGraphicsCommandList()
{
	return m_pGraphicsCommandList;
}
float CD3DManager::GetAspectRatio()
{
	return m_fAspectRatio;
}
ID3D12CommandAllocator* CD3DManager::getCommandAllocator()
{
	return m_pCommandAllocator;
}
ID3D12PipelineState* CD3DManager::getPipelineStateObject()
{
	return m_pPipelineState;
}
ID3D12RootSignature* CD3DManager::getRootSignature()
{
	return m_pRootSignature;
}
/**********************************PRIVATE FUNCTIONS***************************************/
bool CD3DManager::CreateRootDescriptor(HWND in_hWindow)
{
	/********************************************************Create root signature*******************************************************/
	HRESULT isCreated;
	/*Create descriptor range, so we can create our descriptor table in which we will save our constant buffer data*/
	D3D12_DESCRIPTOR_RANGE RangeDesc = {};//just one range 
	/*this describes whether this is a range of srv's, uav's, cbv's, or samplers*/
	RangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	RangeDesc.NumDescriptors = 1; //we only have 1 constant buffer
	RangeDesc.BaseShaderRegister = 0; //each descriptor should bound to one register, this is the first for CB
	RangeDesc.RegisterSpace = 0;
	RangeDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //append the range to the end of the root signature

	/*Fill table using our Range descriptor*/

	D3D12_ROOT_DESCRIPTOR_TABLE rootDescriptorTable;
	rootDescriptorTable.NumDescriptorRanges = 1;//_countof(RangeDesc);//only one range for now
	rootDescriptorTable.pDescriptorRanges = &RangeDesc;

	/*Create Root paramerter*/
	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //this is a descriptor table  root parameter
	rootParameter.DescriptorTable = rootDescriptorTable;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// | D3D12_SHADER_VISIBILITY_PIXEL; //allow vertex and pixel shader to acces the CBV


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	/*Creat empty RooT signature DESC,  to define resource bindings */
	rootSignatureDesc.NumParameters = 1;// _countof(rootParameter);
	rootSignatureDesc.pParameters = &rootParameter;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;

	/*The app is opting in to using the Input Assembler (requiring an input layout that defines a set of vertex buffer bindings).
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_root_signature_flags
	*/
	/*When creating a root parameter, you want to deny any shader stages access to that parameter that do not need it. This allows the GPU to optimize access's to the data.*/
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


	/*Blobs can be used as data buffers. Blobs can also be used for storing vertex, adjacency,
	and material information during mesh optimization, and for loading operations. Also,
	these objects are used to return object code and error messages in APIs that compile vertex, geometry, and pixel shaders.
	*/
	ID3DBlob* pSignature;
	ID3DBlob* pError;

	isCreated = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not Serialize Root Signature.", "DirectX Root Signature failed.", MB_OK);
		return false;
	}
	ZeroMemory(&m_pRootSignature, sizeof(ID3D12RootSignature));
	isCreated = m_pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not create Root Signature.", "DirectX Root Signature failed.", MB_OK);
		return false;
	}
	return true; 
}
void CD3DManager::CreateRasterizer()
{
	/*
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc#:~:text=A%20D3D12_GRAPHICS_PIPELINE_STATE_DESCcontains%20a%20rasterizer-state%20structure.%20Rasterizer%20state%20defines,uses%20the%20following%20default%20values%20for%20rasterizer%20state.
	*/
	m_rasterizerDesc = {};
	m_rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	m_rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	m_rasterizerDesc.FrontCounterClockwise = FALSE;
	m_rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	m_rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	m_rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	m_rasterizerDesc.DepthClipEnable = TRUE;
	m_rasterizerDesc.MultisampleEnable = FALSE;
	m_rasterizerDesc.AntialiasedLineEnable = FALSE;
	m_rasterizerDesc.ForcedSampleCount = 0;
	m_rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
}
void CD3DManager::CreateBlend()
{
	/*	const D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
		renderTargetBlendDesc.BlendEnable = FALSE;
		renderTargetBlendDesc.LogicOpEnable = FALSE;
		renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
		renderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
		renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	*/
	const D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc =
	{
		FALSE,FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};

	m_blendDesc = {};
	m_blendDesc.AlphaToCoverageEnable = FALSE;
	m_blendDesc.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		m_blendDesc.RenderTarget[i] = renderTargetBlendDesc;


}
void CD3DManager::CreateDepthStencil()
{
	D3D12_DEPTH_STENCIL_DESC m_depthStencilDesc;
	ZeroMemory(&m_depthStencilDesc, sizeof(m_depthStencilDesc));
	m_depthStencilDesc.DepthEnable = FALSE;
	m_depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	m_depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	m_depthStencilDesc.StencilEnable = FALSE;
	m_depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	m_depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	m_depthStencilDesc.FrontFace = defaultStencilOp;
	m_depthStencilDesc.BackFace = defaultStencilOp;
}
bool CD3DManager::CreatePSO(HWND in_hWindow)
{
	HRESULT isCreated;
	/***************************************Create the pipeline state object description************************/
	/* A pipelinestate object maintains the state of all currently set shaders as well as certain fixed function state
		objects (such as the input assembler, tesselator, rasterizer and output merger)*/

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeLineStateDescriptor = {};
	pipeLineStateDescriptor.InputLayout = { m_pShaderManager->getInputLayout(), INPUTLAYOUTELEMNUM };
	pipeLineStateDescriptor.pRootSignature = m_pRootSignature;
	pipeLineStateDescriptor.VS = { m_pShaderManager->getVertexShader()->GetBufferPointer(), m_pShaderManager->getVertexShader()->GetBufferSize() };
	pipeLineStateDescriptor.PS = { m_pShaderManager->getPixelShader()->GetBufferPointer(), m_pShaderManager->getPixelShader()->GetBufferSize() };
	pipeLineStateDescriptor.RasterizerState = m_rasterizerDesc;
	pipeLineStateDescriptor.BlendState = m_blendDesc;
	pipeLineStateDescriptor.DepthStencilState = m_depthStencilDesc;
	pipeLineStateDescriptor.SampleMask = UINT_MAX;
	pipeLineStateDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeLineStateDescriptor.NumRenderTargets = 1;
	pipeLineStateDescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipeLineStateDescriptor.SampleDesc.Count = 1;

	isCreated = m_pDevice->CreateGraphicsPipelineState(&pipeLineStateDescriptor, IID_PPV_ARGS(&m_pPipelineState));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not create pipeline state.", "DirectX pipeline state creation failed.", MB_OK);
		return false;
	}
	return true;
}
bool CD3DManager::CreateIndexBuffer(HWND in_hWindow)
{
	HRESULT isCreated;
	///*index BUFFER*/
	DWORD indexBufferData[] = {
		0, 1, 2, // first triangle
		0, 3, 1,//second
	};

	const UINT uiIndexBufferSize = sizeof(indexBufferData);

	D3D12_HEAP_PROPERTIES indexBufferHeapProperties;
	indexBufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	indexBufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	indexBufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	indexBufferHeapProperties.CreationNodeMask = 1;
	indexBufferHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC indexBufferResourceDesc;
	indexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexBufferResourceDesc.Alignment = 0;
	indexBufferResourceDesc.Width = uiIndexBufferSize;
	indexBufferResourceDesc.Height = 1;
	indexBufferResourceDesc.DepthOrArraySize = 1;
	indexBufferResourceDesc.MipLevels = 1;
	indexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	indexBufferResourceDesc.SampleDesc.Count = 1;
	indexBufferResourceDesc.SampleDesc.Quality = 0;
	indexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	indexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	isCreated = m_pDevice->CreateCommittedResource(&indexBufferHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pIndexBuffer));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not create index buffer resource.", "DirectX commited resource creation failed.", MB_OK);
		return false;
	}

	// We do not intend to read from this resource on the CPU.
	D3D12_RANGE Range;
	Range.Begin = 0;
	Range.End = 0;


	isCreated = m_pIndexBuffer->Map(0, &Range, reinterpret_cast<void**>(&puiIndexBufferData));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not map index buffer.", "DirectX index buffer failed.", MB_OK);
		return false;
	}
	// Copy data to DirectX 12 driver memory
	memcpy(puiIndexBufferData, indexBufferData, sizeof(indexBufferData));
	m_pIndexBuffer->Unmap(0, nullptr);

	// Initialize the index buffer view.
	m_indexBufferView.BufferLocation = m_pIndexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = uiIndexBufferSize;

	return true;
}
bool CD3DManager::CreateVertexBufferView(HWND in_hWindow)
{
	HRESULT isCreated;
	/*This function might be in the Model class, and i will just create a get vertexbufferview function
		and a get pVertexData, so i can use them in D3DManager
	*/
	/*Create Geometry*/
	float fAspectRatio = GetAspectRatio();
	/*I should receive this from a class model, which will handles the geometry of my scenario*/
	Vertex triangleVertices[] =
	{
		{ { -0.1f, 0.16f , 0.9f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.1f, -0.16f , 0.9f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.1f, -0.16f, 0.9f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { 0.1f, 0.16f , 0.9f }, { 0.0f, 0.0f, 0.0f, 1.0f } },

	};
	
	/*Create HEAP properties */
	D3D12_HEAP_PROPERTIES heapProperties = {};
	ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.VisibleNodeMask = 1;

	/*Create vertex buffer resource*/
	D3D12_RESOURCE_DESC vertexBufferResource;
	ZeroMemory(&vertexBufferResource, sizeof(D3D12_RESOURCE_DESC));
	vertexBufferResource.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferResource.Alignment = 0;
	vertexBufferResource.Width = sizeof(triangleVertices);
	vertexBufferResource.Height = 1;
	vertexBufferResource.DepthOrArraySize = 1;
	vertexBufferResource.MipLevels = 1;
	vertexBufferResource.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferResource.SampleDesc.Count = 1;
	vertexBufferResource.SampleDesc.Quality = 0;
	vertexBufferResource.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferResource.Flags = D3D12_RESOURCE_FLAG_NONE;
	for (size_t i = 0; i < BufferCount; i++)
	{
		
		isCreated = m_pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferResource, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pShaderBuffer[i]));
		if (FAILED(isCreated))
		{

			MessageBox(in_hWindow, "Could not create cvertex buffer view  resource.", "DirectX commited resource creation failed.", MB_OK);
			return false;
		}

		/*Copy our vertex data to the vertex buffer (mapping)*/
		//UINT8* pVertexDataBegin;
		D3D12_RANGE Range;
		Range.Begin = 0;
		Range.End = 0;
		isCreated = m_pShaderBuffer[i]->Map(0, &Range, reinterpret_cast<void**>(&m_pVertexData));
		if (FAILED(isCreated))
		{

			MessageBox(in_hWindow, "Could not map vertex buffer.", "DirectX vertex buffer failed.", MB_OK);
			return false;
		}
		memcpy(m_pVertexData, &triangleVertices, sizeof(triangleVertices));
		m_pShaderBuffer[i]->Unmap(0, nullptr);

		/*Initialize vertex buffer view*/
		m_VertexBufferView.BufferLocation = m_pShaderBuffer[i]->GetGPUVirtualAddress();
		m_VertexBufferView.StrideInBytes = sizeof(Vertex);
		m_VertexBufferView.SizeInBytes = sizeof(triangleVertices);
	}
	
	return true;
}
bool CD3DManager::LoadPipeline(HWND in_hWindow, int in_nScreenHeight, int in_nScreenWidth, bool in_bVSync, bool in_bFullScreen)
{
	UINT uiDXGIFactoryFlags = 0;
	/*< Value to verify if we could create our device or if something failed. */
	HRESULT isCreated;

	/*<Describes the set of features targeted by a Direct3D device.Set what version of DirectX to be used.*/
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_12_1;

	/*<Describes a command queue, Executing our command lists, All our rendering command are place in a command queue, the gpu executes from that command queue.
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_command_queue_desc
		D3D12_COMMAND_LIST_TYPE   Type, Specifies one member of D3D12_COMMAND_LIST_TYPE (https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type)
		INT                       Priority, The priority for the command queue, as a D3D12_COMMAND_QUEUE_PRIORITY enumeration constant to select normal or high priority. (https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_queue_priority)
		D3D12_COMMAND_QUEUE_FLAGS Flags, Specifies any flags from the D3D12_COMMAND_QUEUE_FLAGS enumeration. (https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_queue_flags)
		UINT                      NodeMask, For single GPU operation, set this to zero. If there are multiple GPU nodes, set a bit to identify the node (the device's physical adapter) to which the command queue applies.
	*/
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;

	/*<Enables creating Microsoft DirectX Graphics Infrastructure (DXGI) objects.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_4/nn-dxgi1_4-idxgifactory4
	*/
	IDXGIFactory4* pFactory;

	/*<The IDXGIAdapter interface represents a display subsystem (including one or more GPUs, DACs and video memory).
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiadapter
	*/
	IDXGIAdapter* pAdapter;

	/*<An IDXGIOutput interface represents an adapter output (such as a monitor).
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgioutput
	*/
	IDXGIOutput* pAdapterOutput;
	int uiError;
	UINT uiNumModes, uiIterator, uiNumerator, uiDenominator;
	unsigned long long ullStringLength;
	uiNumModes = uiIterator = uiNumerator = uiDenominator = uiError = ullStringLength = 0;
	/*<Describes a display mode.
			UINT Width, A value that describes the resolution width.
			UINT Height, A value describing the resolution height.
			DXGI_RATIONAL RefreshRate, Structure describing the refresh rate in hertz. (https://docs.microsoft.com/en-us/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_rational)
			DXGI_FORMAT Format, Structure describing the display format. (https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format)
			DXGI_MODE_SCANLINE_ORDER ScanlineOrdering, Enumerated type describing the scanline drawing mode, method the raster uses to create an image on a surface.. (https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173067%28v%3dvs.85%29)
			DXGI_MODE_SCALING   Scaling, Enumerated type describing the scaling mode, how an image is stretched to fit a given monitor's resolution.(https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173066%28v%3dvs.85%29)
			(https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173064(v%3Dvs.85)
	*/
	DXGI_MODE_DESC* pDisplayModeList;

	/*<Describes an adapter (or video card)
	WCHAR  Description[128], A string that contains the adapter description.
	UINT   VendorId, The PCI ID of the hardware vendor.
	UINT   DeviceId, The PCI ID of the hardware device.
	UINT   SubSysId, The PCI ID of the sub system.
	UINT   Revision, The PCI ID of the revision number of the adapter.
	SIZE_T DedicatedVideoMemory, The number of bytes of dedicated video memory that are not shared with the CPU.
	SIZE_T DedicatedSystemMemory, The number of bytes of dedicated system memory that are not shared with the CPU.
	SIZE_T SharedSystemMemory, The number of bytes of shared system memory.
	LUID   AdapterLuid, A unique value that identifies the adapter.
	https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_adapter_desc
*/
	DXGI_ADAPTER_DESC AdapterDesc;

	/*<Describes a swap chain.
		DXGI_MODE_DESC   BufferDesc, A DXGI_MODE_DESC structure that describes the backbuffer display mode.
		DXGI_SAMPLE_DESC SampleDesc, A DXGI_SAMPLE_DESC structure that describes multi-sampling parameters.(https://docs.microsoft.com/en-us/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_sample_desc)
		DXGI_USAGE       BufferUsage, A member of the DXGI_USAGE enumerated type that describes the surface usage and CPU access options for the back buffer. The back buffer can be used for shader input or render-target output.
		UINT             BufferCount, A value that describes the number of buffers in the swap chain.
		HWND             OutputWindow, An HWND handle to the output window.
		BOOL             Windowed, A Boolean value that specifies whether the output is in windowed mode.
		DXGI_SWAP_EFFECT SwapEffect, A member of the DXGI_SWAP_EFFECT enumerated type that describes options for handling the contents of the presentation buffer after presenting a surface.Options for handling pixels in a display surface (https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect)
		UINT             Flags,  member of the DXGI_SWAP_CHAIN_FLAG enumerated type that describes options for swap-chain behavior.(https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag)
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc
	*/
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	/*< Interface implements one or more surfaces for storing rendered data before presenting it to an output.
		Methods:
		https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiswapchain
	*/
	IDXGISwapChain* pSwapChain;

	/*<Describes the descriptor heap.*/
	D3D12_DESCRIPTOR_HEAP_DESC RenderTargetViewHeapDesc = {};

	/*<Describes the CPU decriptor handle.
		SIZE_T ptr	: The address of the descriptor.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle;

	/*<Store Vertical Synchronization setting*/
	m_bIsVSyncEnabled = in_bVSync;

	/*Enable Debug layer */
	#if defined(_DEBUG)
	{
		
		ID3D12Debug* pDebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();
		
			/*Enable additional debug layers*/
			uiDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}

	}
	#endif


		/*Create DirectX graphics interface factory
	*/
	isCreated = CreateDXGIFactory2(uiDXGIFactoryFlags, IID_PPV_ARGS(&pFactory));
	if (FAILED(isCreated))
	{
		/*< Failed create Factory.*/
		MessageBox(in_hWindow, "Could not create a DirectX 12 Factory", "DirectX Factory failure", MB_OK);
		return false;
	}
	/*Use factory to create an adapter for the primary graphics interface.
		UINT         Adapter, The index of the adapter to enumerate.
		IDXGIAdapter **ppAdapter, The address of a pointer to an IDXGIAdapter interface at the position specified by the Adapter parameter.
	*/
	isCreated = pFactory->EnumAdapters(0, &pAdapter);
	if (FAILED(isCreated))
	{
		return false;
	}
	/*Enumerate the primary adapter output
		UINT        Output, The index of the output.
		IDXGIOutput **ppOutput, The address of a pointer to an IDXGIOutput interface at the position specified by the Output parameter.
	*/
	isCreated = pAdapter->EnumOutputs(0, &pAdapterOutput);
	if (FAILED(isCreated))
	{
		return false;
	}
	/*Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
		  DXGI_FORMAT     EnumFormat, A DXGI_FORMAT-typed value for the color format.
			 DXGI_FORMAT_R8G8B8A8_UNORM, A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha.(https://docs.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format)
		  UINT            Flags, Specifies options for display modes to include.
			 DXGI_ENUM_MODES_INTERLACED, Include interlaced modes.(https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-enum-modes)
		  UINT            *pNumModes, A pointer to a variable that receives the number of display modes thatreturns in the memory block to which pDesc points.Set pDesc to NULL so that pNumModes returns the number of display modes that match the format and the options.
		  DXGI_MODE_DESC1 *pDesc, A pointer to a list of display modes; set to NULL to get the number of display modes.
		  https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutput1-getdisplaymodelist1
	*/
	isCreated = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, NULL);
	if (FAILED(isCreated))
	{
		return false;
	}
	/*Create a list to hold all the displays modes for the monitor/videcard combination.*/
	pDisplayModeList = new DXGI_MODE_DESC[uiNumModes];
	if (!pDisplayModeList)

	{
		return false;
	}
	/*Fill display mode list structures*/
	isCreated = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, pDisplayModeList);
	if (FAILED(isCreated))
	{
		return false;
	}
	/* Get throgh all the display modes and fin the one matching height and width, then get the refresh rate and stored them in numerator and denominatior. */
	for (uiIterator = 0; uiIterator < uiNumModes; uiIterator++)
	{
		if (pDisplayModeList[uiIterator].Height == (unsigned int)in_nScreenHeight)
		{
			if (pDisplayModeList[uiIterator].Width == (unsigned int)in_nScreenWidth)
			{
				uiDenominator = pDisplayModeList[uiIterator].RefreshRate.Denominator;
				uiNumerator = pDisplayModeList[uiIterator].RefreshRate.Numerator;
			}
		}
	}
	/*Get adapter (video card) description
		DXGI_ADAPTER_DESC *pDesc, A pointer to a DXGI_ADAPTER_DESC structure that describes the adapter.
	*/
	
	isCreated = pAdapter->GetDesc(&AdapterDesc);
	if (FAILED(isCreated))
	{
		return false;
	}

	/*Store videocard memory in mega bytes*/
	m_nVideoCardMemory = (int)(AdapterDesc.DedicatedVideoMemory / KB / KB);
	/* Convert videocard's name to a character array.
		wcstombs_s, Converts a sequence of wide characters to a corresponding sequence of multibyte characters.
	*/
	uiError = wcstombs_s(&ullStringLength, m_cVideoCardDescription, VideoCardDescSize, AdapterDesc.Description, 128);
	if (uiError)
	{
		return false;
	}

	/*Release Display, adapater and adapater descriptor, since its important information is already stored. */
	delete[] pDisplayModeList;
	pDisplayModeList = 0;
	pAdapterOutput->Release();
	pAdapterOutput = 0;
	pAdapter->Release();
	pAdapter = 0;

	/*******************************************Create Direct3D 12 Device****************************************

	D3D12CreateDevice : Creates a device that represents the display adapter.
	IUnknown          *pAdapter, A pointer to the video adapter to use when creating a device. NUll to use default adapter
	D3D_FEATURE_LEVEL MinimumFeatureLevel, The minimum D3D_FEATURE_LEVEL required for successful device creation.
	REFIID            riid, The globally unique identifier (GUID) for the device interface.
	void              **ppDevice, A pointer to a memory block that receives a pointer to the device.

	$ __uuidof : Retrieves the GUID attached to the expression.
	https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
*/
	isCreated = D3D12CreateDevice(pAdapter, FeatureLevel, IID_PPV_ARGS(&m_pDevice));
	if (FAILED(isCreated))
	{
		/*< Failed create device.*/
		MessageBox(in_hWindow, "Could not create a DirectX 12.1 device.  The default video card does not support DirectX 12.1.", "DirectX Device Failure", MB_OK);
		return false;
	}
	/*******************************************Create Command queue****************************************
	/*Initialize Command queue description.*/
	ZeroMemory(&CommandQueueDesc, sizeof(CommandQueueDesc));

	/*Set up command queue descriptor
		Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
	*/
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	/*Normal priority.*/
	CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	/*Indicates a default command queue.*/
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	/*Use only one GPU.*/
	//CommandQueueDesc.NodeMask = 0;


	/*Create command queue.
		  const D3D12_COMMAND_QUEUE_DESC *pDesc, Specifies a D3D12_COMMAND_QUEUE_DESC that describes the command queue.
		  REFIID riid, The globally unique identifier (GUID) for the command queue interface. See remarks. An input parameter.
		  void  **ppCommandQueue, A pointer to a memory block that receives a pointer to the ID3D12CommandQueue interface for the command queue.
		https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommandqueue
	*/
	isCreated = m_pDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS (&m_pCommandQueue));

	if (FAILED(isCreated))
	{
		/*< Failed create Command queue.*/
		MessageBox(in_hWindow, "Could not create a Command queue", "DirectX Command queue failure", MB_OK);
		return false;
	}

	/*******************************************************Create Swap Chain****************************/
	

	/*Initialize Swap Chain descriptor memory.*/
	ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));

	/*Set swap chain to use Predefined buffer count.*/
	SwapChainDesc.BufferCount = BufferCount;

	/*Set width and height from the swap chain's back buffers. */
	SwapChainDesc.BufferDesc.Height = in_nScreenHeight;
	SwapChainDesc.BufferDesc.Width = in_nScreenWidth;

	/*Set a regular 32-bit surface for the back buffers.*/
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	/*Set the usage of the back buffers to be render target outputs. */
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	/* Discard the previous buffer contents after swapping. */
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	/* Set the handle for the window to render to.*/
	SwapChainDesc.OutputWindow = in_hWindow;


	/*Turn off multisampling*/
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;

	/*Set fullscreen or windowed.*/
	if (in_bFullScreen)
	{
		SwapChainDesc.Windowed = false;
	}
	else
	{
		SwapChainDesc.Windowed = true;
	}

	/*Set refresh rate (How many times per second it renders the back buffer)*/
	if (m_bIsVSyncEnabled)
	{
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = uiDenominator;
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = uiNumerator;
	}
	else
	{
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	}

	/*Set Scanline ordering and scaling to unspecified*/
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	/*Not setting advanced flags*/
	SwapChainDesc.Flags = 0;

	/*Create the swap chain using swap chain descriptor*/
	isCreated = pFactory->CreateSwapChain(m_pCommandQueue, &SwapChainDesc, &pSwapChain); //old one
	if (FAILED(isCreated))
	{
		/*< Failed create Swap Chain.*/
		MessageBox(in_hWindow, "Could not create a Swap Chain", "DirectX SwapChain failure", MB_OK);
		return false;
	}

	/*Change from IDXGISwapChain to a IDXGISwapChain3 interface, to get newer functions such as get the back buffer index*/
	isCreated = pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&m_pSwapChain);
	if (FAILED(isCreated))
	{
		MessageBox(in_hWindow, "Could not queryIntarface for Swap Chain", "DirectX SwapChain failure", MB_OK);
		return false;
	}
	/*Clear original swap chain's pointer.*/
	pSwapChain = 0;
	/*Release factory*/
	pFactory->Release();
	pFactory = 0;

	/***************************************************Create Render target view*************************/
	/*Initialize render target view heap descriptor*/
	ZeroMemory(&RenderTargetViewHeapDesc, sizeof(RenderTargetViewHeapDesc));

	/*Get initial index to where buffer is currently back buffer*/
	m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	/*Set number of  descriptor for our 2 back buffers*/
	RenderTargetViewHeapDesc.NumDescriptors = BufferCount;
	/*Set render target descriptor type, The descriptor heap for the render-target view. Default use for descriptor*/
	RenderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RenderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	/*Create render target view heap for the back buffers */
	isCreated = m_pDevice->CreateDescriptorHeap(&RenderTargetViewHeapDesc, IID_PPV_ARGS(&m_pRenderTargetViewHeap));
	if (FAILED(isCreated))
	{
		MessageBox(in_hWindow, "Could not create DescriptorHeap", "DirectX Descriptor heap failure", MB_OK);
		return false;
	}

	/*Get size of the memory location for the render target view descriptor*/
	m_uiRenderTargetViewDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	/*Get a hanlde from the starting memory location in the render target view heap to identify where the render target views will be located for the 2 back buffers*/
	RenderTargetViewHandle = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

	/****************************************************Create Frame Resources*****************************/
	
	
	/*Create RTV for each buffer*/
	for (UINT nCounter = 0; nCounter < BufferCount; nCounter++)
	{
		/*Get a pointer to the buffer from the swap chain*/
		isCreated = m_pSwapChain->GetBuffer(nCounter, __uuidof(ID3D12Resource), (void**)&m_pBackBufferRenderTarget[nCounter]);
		if (FAILED(isCreated))
		{
			MessageBox(in_hWindow, "Could not get buffer from Swap Chain", "DirectX SwapChain failure", MB_OK);
			return false;
		}
		/*Create a render target view for the  back buffer
		ID3D12Resource *pResource, A pointer to the ID3D12Resource object that represents the render target.
		const D3D12_RENDER_TARGET_VIEW_DESC *pDesc, A pointer to a D3D12_RENDER_TARGET_VIEW_DESC structure that describes the render-target view.A null pDesc is used to initialize a default descriptor.
		D3D12_CPU_DESCRIPTOR_HANDLE         DestDescriptor	describes the CPU descriptor handle that represents the destination where the newly-created render target view will reside.
		https://docs.microsoft.com/es-es/windows/win32/api/d3d12/nf-d3d12-id3d12device-createrendertargetview
		*/
		m_pDevice->CreateRenderTargetView(m_pBackBufferRenderTarget[nCounter], nullptr, RenderTargetViewHandle);
		/*Increment the view handle to the descriptor location in the RTV heap*/
		RenderTargetViewHandle.ptr = SIZE_T((INT64)RenderTargetViewHandle.ptr + (INT64)1 * (INT64)m_uiRenderTargetViewDescriptorSize);
	}



	/**************************************************Create Command Allocator**************************/
	/*Create command allocator, this allocates memory for the commands sent to GPU.*/
	isCreated = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
	if (FAILED(isCreated))
	{
		MessageBox(in_hWindow, "Could not create command allocator", "DirectX Command allocator failure", MB_OK);
		return false;
	}
	return true;
}
bool CD3DManager::CreateFence(HWND in_hWindow) 
{
	HRESULT isCreated;
	/*Create fence, synchronization object and wait until assets have been uploaded to the gpu*/
	isCreated = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not create fence.", "DirectX fence creation failed.", MB_OK);
		return false;
	}

	m_ullFenceValue = 1;

	/*create event handle to use frame synchronization*/

	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (FAILED(m_hFenceEvent))
	{

		MessageBox(in_hWindow, "Could not create fence event.", "DirectX fence event creation failed.", MB_OK);
		return false;
	}
	/*wait for command list to execute
		list in our main loop for now, waitr for set up before continuing
		*/
	WaitForPreviousFrame(in_hWindow);
	return true;
}
bool CD3DManager::LoadAssets(HWND in_hWindow, WCHAR* in_pVertexShader, WCHAR* in_pPixelShader)
{
	HRESULT isCreated;
	if (!CreateRootDescriptor(in_hWindow))
	{
		/*If root descriptor fails to create, */
		return false;
	}
	if (!m_pShaderManager->OpenShader(in_hWindow, in_pVertexShader, in_pPixelShader))
	{
		return false;
	}
	m_pShaderManager->CreateInputLayout();
	CreateRasterizer();
	CreateBlend();
	CreateDepthStencil();
	CreatePSO(in_hWindow);
	CreateVertexBufferView(in_hWindow);
	CreateIndexBuffer(in_hWindow);
	m_pShaderManager->InitConstBuffer(m_pDevice, in_hWindow);
	//InitConstBuffer(in_hWindow);

	/*Create commmand list ( this should be in another class in the future), command list is a list of all the rendering commands to be executed, */
	isCreated = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, m_pPipelineState, IID_PPV_ARGS(&m_pGraphicsCommandList));
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not create command list.", "DirectX command list failed.", MB_OK);
		return false;
	}
	/*It is needed to close the command list while initializing as is created in recording state*/
	isCreated = m_pGraphicsCommandList->Close();
	if (FAILED(isCreated))
	{
		MessageBox(in_hWindow, "Could not close command list.", "DirectX close command list failed.", MB_OK);
		return false;
	}

	CreateFence(in_hWindow);
	return true;
}
bool CD3DManager::PopulateCommandList(HWND in_hWindow)
{

	HRESULT isSuccess;

	/*Describes a resource barrier
	https://docs.microsoft.com/es-es/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_barrier
	*/
	D3D12_RESOURCE_BARRIER ResourceBarrier;

	/*
		Describes a CPU descriptor handle.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle;

	/*Reset  the memory associated command allocator*/
	isSuccess = m_pCommandAllocator->Reset();
	if (FAILED(isSuccess))
	{
		return false;
	}

	///*Reset the command list, use empty pipeline state for now since there are no shaders and we are just clearing the screen. TODO
	//Resets a command list back to its initial state as if a new command list was just created.*/
	isSuccess = m_pGraphicsCommandList->Reset(m_pCommandAllocator, m_pPipelineState);
	if (FAILED(isSuccess))
	{
		return false;
	}

	m_pGraphicsCommandList->SetGraphicsRootSignature(m_pRootSignature);
	/*ADD CBUffer HEAPS*/
	for (int i = 0; i < BufferCount; i++)
	{

		ID3D12DescriptorHeap* ppHeaps[] = { m_pShaderManager->getConstBufferViewHeap()[i] };
		m_pGraphicsCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		m_pGraphicsCommandList->SetGraphicsRootDescriptorTable(0, m_pShaderManager->getConstBufferViewHeap()[i]->GetGPUDescriptorHandleForHeapStart());

	}
	// Set necessary state.
	m_pGraphicsCommandList->RSSetViewports(1,&m_Viewport);
	m_pGraphicsCommandList->RSSetScissorRects(1, &m_scissorRect);

	/*Set the resource barrier. For synchronize the next back buffer.
		Flags:
		https://docs.microsoft.com/es-es/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_barrier_flags
	*/
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Transition.pResource = m_pBackBufferRenderTarget[m_uiFrameIndex];
	/*a resource must be in the COMMON state before being used on a COPY queue */
	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	/*The resource is used as a render target.*/
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	/*to transition all subresources in a resource at the same time.*/
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	/*A transition barrier that indicates a transition of a set of subresources between different usages. */
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	/*Record commands in the command list*/
	m_pGraphicsCommandList->ResourceBarrier(1, &ResourceBarrier);

	/*Get the render target view handle for the current back buffer.*/
	RenderTargetViewHandle = m_pRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	RenderTargetViewHandle.ptr = SIZE_T((INT64)RenderTargetViewHandle.ptr + (INT64)m_uiFrameIndex * (INT64)m_uiRenderTargetViewDescriptorSize);
	

	/* Set the back buffer as the render target.
		Sets CPU descriptor handles for the render targets and depth stencil.
		  UINT NumRenderTargetDescriptors, The number of entries in the pRenderTargetDescriptors array.
		  const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,Specifies an array of D3D12_CPU_DESCRIPTOR_HANDLE structures that describe the CPU descriptor handles that represents the start of the heap of render target descriptors.
		  BOOL RTsSingleHandleToDescriptorRange,True means the handle passed in is the pointer to a contiguous range of NumRenderTargetDescriptors descriptors. False means that the handle is the first of an array of NumRenderTargetDescriptors handles.
		  const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor A pointer to a D3D12_CPU_DESCRIPTOR_HANDLE structure that describes the CPU descriptor handle that represents the start of the heap that holds the depth stencil descriptor.
		  https://docs.microsoft.com/es-es/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-omsetrendertargets
	*/


	m_pGraphicsCommandList->OMSetRenderTargets(1, &RenderTargetViewHandle, FALSE, nullptr);
	const float clearColor[] = { 0.1f, 0.7f, 0.4f, 1.0f };
	/*Clear Render target, and submit that command to the commnd list*/
	m_pGraphicsCommandList->ClearRenderTargetView(RenderTargetViewHandle, clearColor, 0, nullptr);
	
	m_pGraphicsCommandList->SetGraphicsRootSignature(m_pRootSignature);
	/*Set vertex buffer*/
	//set topology
	m_pGraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	/*Set vertex shader*/
	m_pGraphicsCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_pGraphicsCommandList->IASetIndexBuffer(&m_indexBufferView);
	/*Draws non-indexed, instanced primitives.*/
	m_pGraphicsCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	/*ShaderManager  Render*/

	/* Indicate that the back buffer will now be used to present.*/
	ResourceBarrier.Transition.pResource = m_pBackBufferRenderTarget[m_uiFrameIndex];
	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	/*Record commands in the command list*/
	m_pGraphicsCommandList->ResourceBarrier(1, &ResourceBarrier);
	/*Close the list of commands.*/
	isSuccess = m_pGraphicsCommandList->Close();
	if (FAILED(isSuccess))
	{
		return false;
	}
	
	return true;
}
bool CD3DManager::WaitForPreviousFrame(HWND in_hWindow)
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 nFenceWaitFor = m_ullFenceValue;
	HRESULT isCreated;

	isCreated = m_pCommandQueue->Signal(m_pFence, nFenceWaitFor);
	if (FAILED(isCreated))
	{
		MessageBox(in_hWindow, "Could not Signal the fence.", "DirectX Fence signal failed", MB_OK);
		return false;
	}
	m_ullFenceValue++;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < nFenceWaitFor)
	{
		/*Specifies an event that should be fired when the fence reaches a certain value.*/
		isCreated = m_pFence->SetEventOnCompletion(nFenceWaitFor, m_hFenceEvent);
		if (FAILED(isCreated))
		{
			return false;
		}
		/*Waits until the specified object is in the signaled state or the time-out interval elapses.*/
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
	m_uiFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	return true;
}
