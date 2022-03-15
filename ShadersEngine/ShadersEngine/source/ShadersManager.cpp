#include "../headers/ShadersManager.h"

CShadersManager::CShadersManager()
{
	m_pVertexShader = 0;
	m_pPixelShader = 0;
	for (size_t i = 0; i < BufferCount; i++)
	{
		m_pShaderBuffer[i] = 0;
		m_pConstantBufferViewHeap[i] = 0;
	}

	//uiBufferNumber = 0;

	m_puiConstantBufferBeginData = nullptr;
}
CShadersManager::~CShadersManager()
{
}

bool CShadersManager::OpenShader(HWND in_hWindow, WCHAR* in_pVertexShader, WCHAR* in_pPixelShader)
{
	/*If pixel shader is empty it means its the same as the vertexshader hlsl*/
	in_pPixelShader = in_pPixelShader ? in_pPixelShader : in_pVertexShader;
	HRESULT isCreated;
	UINT uiCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	uiCompileFlags |= D3DCOMPILE_DEBUG;
#else
	 uiCompileFlags = 0;
#endif
	/*Compile Vertex and pixel shaders*/
isCreated = D3DCompileFromFile(in_pVertexShader, nullptr, nullptr, "VSMain", "vs_5_0", uiCompileFlags, 0, &m_pVertexShader, nullptr);
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not Compile Vertex Shader.", "DirectX VertexShader Compiled failed.", MB_OK);
		return false;
	}
	isCreated = D3DCompileFromFile(in_pPixelShader, nullptr, nullptr, "PSMain", "ps_5_0", uiCompileFlags, 0, &m_pPixelShader, nullptr);
	if (FAILED(isCreated))
	{

		MessageBox(in_hWindow, "Could not Compile Pixel  Shader.", "DirectX Pixelshader Compiled failed.", MB_OK);
		return false;
	}
	return true;
}
void CShadersManager::CreateInputLayout()
{
	D3D12_INPUT_ELEMENT_DESC InputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	m_pInputLayout[0] = InputLayout[0];
	m_pInputLayout[1] = InputLayout[1];
	
}
bool CShadersManager::InitConstBuffer(ID3D12Device* in_pDevice, HWND in_hWindow)
{
	HRESULT isCreated;
	
	for (int i = 0; i < BufferCount; i++)
	{

		D3D12_DESCRIPTOR_HEAP_DESC ConstanBufferHeapDesc = {};
		/*how many descriptor we will have for this*/
		ConstanBufferHeapDesc.NumDescriptors = 1;
		//t is be bound on a command list for reference by shaders
		ConstanBufferHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		/*The descriptor heap for the combination of constant-buffer, shader-resource, and unordered-access views.*/
		ConstanBufferHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		isCreated = in_pDevice->CreateDescriptorHeap(&ConstanBufferHeapDesc, IID_PPV_ARGS(&m_pConstantBufferViewHeap[i]));
		if (FAILED(isCreated))
		{
			MessageBox(in_hWindow, "Could not create Constant buffer heap .", "DirectX heap creation failed.", MB_OK);
			return false;
		}
	}
	/*Create constant buffer*/

	const UINT uiConstantBufferSize = sizeof(m_ConstantBufferData);
	/*Create HEAP properties */
	D3D12_HEAP_PROPERTIES heapProperties = {};
	ZeroMemory(&heapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.VisibleNodeMask = 1;

	/*Create Constant buffer resource*/
	D3D12_RESOURCE_DESC ConstantBufferResource;
	ZeroMemory(&ConstantBufferResource, sizeof(D3D12_RESOURCE_DESC));
	ConstantBufferResource.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ConstantBufferResource.Alignment = 0;
	ConstantBufferResource.Width = uiConstantBufferSize;
	ConstantBufferResource.Height = 1;
	ConstantBufferResource.DepthOrArraySize = 1;
	ConstantBufferResource.MipLevels = 1;
	ConstantBufferResource.Format = DXGI_FORMAT_UNKNOWN;
	ConstantBufferResource.SampleDesc.Count = 1;
	ConstantBufferResource.SampleDesc.Quality = 0;
	ConstantBufferResource.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ConstantBufferResource.Flags = D3D12_RESOURCE_FLAG_NONE;
	for (size_t i = 0; i < BufferCount; i++)
	{
		isCreated = in_pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &ConstantBufferResource, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ConstantBufferResource));
		if (FAILED(isCreated))
		{
			MessageBox(in_hWindow, "Could not Constant buffer resource.", "DirectX Resource creation failed.", MB_OK);
			return false;
		}

		/*Describe and create constant buffer view*/
		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc = {};
		ConstantBufferViewDesc.BufferLocation = m_ConstantBufferResource->GetGPUVirtualAddress();
		ConstantBufferViewDesc.SizeInBytes = (sizeof(uiConstantBufferSize) + 255) & ~255;
		in_pDevice->CreateConstantBufferView(&ConstantBufferViewDesc, m_pConstantBufferViewHeap[i]->GetCPUDescriptorHandleForHeapStart());
		
		//Set to zero the memory
		ZeroMemory(&m_ConstantBufferData, sizeof(m_ConstantBufferData));
		
		/*we don't want to read from the CPU*/
		D3D12_RANGE Range;
		Range.Begin = 0;
		Range.End = 0;
		isCreated = m_ConstantBufferResource->Map(0, &Range, reinterpret_cast<void**>(&m_puiConstantBufferBeginData));
		memcpy(m_puiConstantBufferBeginData, &m_ConstantBufferData, sizeof(m_ConstantBufferData));
	}

	
	
	return true;
}
void CShadersManager::Exit()
{
	if (m_pVertexShader)
	{
		m_pVertexShader->Release();
		m_pVertexShader = 0;
	}
	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
		m_pPixelShader = 0;
	}

		for (int i = 0; i < BufferCount; i++)
		{
			if (m_pConstantBufferViewHeap[i])
			{
				m_pConstantBufferViewHeap[i]->Release();
				m_pConstantBufferViewHeap[i] = 0;
			}

			if (m_pShaderBuffer)
			{

				//m_pShaderBuffer[i]->Release();
				m_pShaderBuffer[i] = 0;
			}
		
		}
	
	if (m_ConstantBufferResource)
	{
		m_ConstantBufferResource->Release();
		m_ConstantBufferResource = 0;
	}	

}
void CShadersManager::SetShaderConstantBuffer(XMMATRIX in_WorldMatrix, XMMATRIX in_ViewMatrix, XMMATRIX in_ProjectionMatrix)
{

	/*Transpose matrixes to sending to directx so it can work better*/
	in_WorldMatrix = XMMatrixTranspose(in_WorldMatrix);
	in_ViewMatrix = XMMatrixTranspose(in_ViewMatrix);
	in_ProjectionMatrix = XMMatrixTranspose(in_ProjectionMatrix);
	//assing it to my constant buffer
	m_ConstantBufferData.world = in_WorldMatrix;
	m_ConstantBufferData.view = in_ViewMatrix;
	m_ConstantBufferData.projection = in_ProjectionMatrix;

	//update constant buffer data
	memcpy(m_puiConstantBufferBeginData, &m_ConstantBufferData, sizeof(m_ConstantBufferData));

}

ID3DBlob* CShadersManager::getShader(int in_nSelectedShader)
{
	ID3DBlob* pSelectedShader;
	switch (in_nSelectedShader)
	{
	case 1:
		pSelectedShader = m_pVertexShader;
		break;
	case 2:
		pSelectedShader = m_pPixelShader;
		break;
	default:
		pSelectedShader = m_pVertexShader;
		break;
	}
	return pSelectedShader;
}
ID3DBlob* CShadersManager::getVertexShader()
{
	return m_pVertexShader;
}
ID3DBlob* CShadersManager::getPixelShader()
{
	return m_pPixelShader;
}
void	CShadersManager::getVertexShader(ID3DBlob* io_pVertexShader)
{
	io_pVertexShader = m_pVertexShader;
}
void	CShadersManager::getPixelShader(ID3DBlob* io_pPixelShader)
{
	io_pPixelShader = m_pPixelShader;
}
D3D12_INPUT_ELEMENT_DESC* CShadersManager::getInputLayout()
{
	return m_pInputLayout;
}
void CShadersManager::getInputLayout(D3D12_INPUT_ELEMENT_DESC* io_pInputLayout)
{
	io_pInputLayout = m_pInputLayout;
}
ID3D12DescriptorHeap** CShadersManager::getConstBufferViewHeap()
{
	return m_pConstantBufferViewHeap;
}