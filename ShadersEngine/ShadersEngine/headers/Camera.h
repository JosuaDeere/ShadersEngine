#ifndef _CAMERA_H_
#define _CAMERA_H_

#define RADVALUE 0.0174532925f
#include <DirectXMath.h>
using namespace DirectX;
class Camera
{
private:
	//Camera world position X Y Z
	float m_fPositionX;
	float m_fPositionY;
	float m_fPositionZ;

	//Camera rotation X Y Z
	float m_fRotationX;
	float m_fRotationY;
	float m_fRotationZ;

	//Camera view matrix, the view matrix is used to calculate the position of where we are looking at the scene from
	XMMATRIX m_viewMatrix;

public:
	//Camera Constructors
	Camera();
	Camera(const Camera &io_Camera);
	~Camera();

	//Encapsulation functions for Position and rotation.
	void SetPosition(float in_X, float in_Y, float in_Z);
	void SetRotation(float in_X, float in_Y, float in_Z);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	//functions to get view matrix
	XMMATRIX GetViewMatrix();
	void GetViewMatrix(XMMATRIX& io_viewMatrix);

	//Render function which will create the view matrix based on position and rotation.
	void Render();
	
};

#endif