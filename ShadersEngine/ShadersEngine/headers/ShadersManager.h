#ifndef _SHADERSMANAGER_H_
#define _SHADERSMANAGER_H_

#include "LibrariesAndHeaders.h"

/*Parent class from which all the shaders/FX will heritate*/
class CShadersManager 
{
public:
	CShadersManager();
	~CShadersManager();
	/*Initialize vertex and pixel shader, as well as the constant buffer and inputlayout*/
	bool OpenShader(HWND in_hWindow, WCHAR* in_pVertexShader, WCHAR* in_pPixelShader = nullptr);
	/*We send the updated constant buffer to render the triangle*/
	void CreateInputLayout();
	/*safely release all the data.*/
	void Exit();
 
	/*Makes the global shading variables setting*/
	ID3DBlob* getShader(int in_nSelectedShader);
	ID3DBlob* getVertexShader();
	ID3DBlob* getPixelShader();
	D3D12_INPUT_ELEMENT_DESC* getInputLayout();
	void getVertexShader(ID3DBlob* io_pVertexShader);
	void getPixelShader(ID3DBlob* io_pPixelShader);
	void getInputLayout(D3D12_INPUT_ELEMENT_DESC* io_pInputLayout);
	void SetShaderConstantBuffer(XMMATRIX in_WorldMatrix, XMMATRIX in_ViewMatrix, XMMATRIX in_ProjectionMatrix);
	bool InitConstBuffer(ID3D12Device* in_pDevice, HWND in_hWindow);
	ID3D12DescriptorHeap** getConstBufferViewHeap();
protected:

	/*Pointers to my principal shaders, derived classes will add more objects like Geomtry shader, hull shader etc.*/
	ID3DBlob* m_pVertexShader;
	ID3DBlob* m_pPixelShader;
	/*Descriptor heap for our Constant buffer, so it can be bind to the pipeline state object*/
	ID3D12DescriptorHeap* m_pConstantBufferViewHeap[BufferCount];
		/*Resource to help us map our vertex data*/
		ID3D12Resource* m_pShaderBuffer[BufferCount];
		/*Constant buffer resource*/
		ID3D12Resource* m_ConstantBufferResource;
		/*Constant buffer data we'll send to the GPU*/
		ConstantBufferMatrix m_ConstantBufferData;
	
		/*Start of our Constant buffer*/
		UINT* m_puiConstantBufferBeginData;
		/*Vertex input layout, is how are we going to send the vertex and color information*/
		D3D12_INPUT_ELEMENT_DESC m_pInputLayout[INPUTLAYOUTELEMNUM];

		/*this is a pointer to the memory location we get, when we map our constant buffer*/
		UINT* uiConstantBufferLocation[BufferCount];
};

#endif


