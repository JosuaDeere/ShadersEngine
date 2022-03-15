#ifndef _LIBRARIESANDHEADERS_H_
#define _LIBRARIESANDHEADERS_H_
//#include "GraphicManager.h"
#define PI  3.141592654f

#define INPUTLAYOUTELEMNUM 2 
constexpr auto KB = 1024;
constexpr auto VideoCardDescSize = 128;
constexpr auto BufferCount = 2;
/*<Headers that include the previous libraries.*/
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

struct Vertex
{
	// 2 VECTORS ONE OF 3D AND OTHER 4D
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};
/*This is my constant buffer of matrixes, for derive clases we can expand it*/
struct ConstantBufferMatrix
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX offset;
	//float offset[208]; // Padding so the constant buffer is 256-byte aligned.
};
static_assert((sizeof(ConstantBufferMatrix) % 256) == 0, "Constant Buffer size must be 256-byte aligned");
#endif
