#include "../headers/Camera.h"

Camera::Camera()
{
	//init all in zero
	m_fPositionX = 0;
	m_fPositionY = 0;
	m_fPositionZ = 0;
	m_fRotationX = 0;
	m_fRotationY = 0;
	m_fRotationZ = 0;

}

Camera::Camera(const Camera& io_Camera)
{
	//is it necesary?
}

Camera::~Camera()
{

}
void Camera::SetPosition(float in_X, float in_Y, float in_Z)
{
	m_fPositionX = in_X;
	m_fPositionY = in_Y;
	m_fPositionZ = in_Z;
}
void Camera::SetRotation(float in_X, float in_Y, float in_Z)
{
	m_fRotationX = in_X;
	m_fRotationY = in_Y;
	m_fRotationZ = in_Z;
}

XMFLOAT3 Camera::GetPosition()
{
	return { m_fPositionX, m_fPositionY, m_fPositionZ };
}
XMFLOAT3 Camera::GetRotation()
{
	return { m_fRotationX, m_fRotationY, m_fRotationZ };
}

XMMATRIX Camera::GetViewMatrix()
{
	return m_viewMatrix;
}
void Camera::GetViewMatrix(XMMATRIX& io_viewMatrix)
{
	io_viewMatrix = m_viewMatrix;
}

void Camera::Render()
{
	//Rotation values for X axis, Y Axis, and Z axis respectevly
	float fPitch, fYaw, fRoll;
	// Vector values for up vector, where the camera is watchig *lookat, and the camera position.
	XMFLOAT3 f3Up, f3LookAt, f3Position;
	//Actual vectors
	XMVECTOR vUp, vLookAt, vPosition;
	//View Matrix
	XMMATRIX rotationMatrix;

	//set Up Vector values
	f3Up.x = 0.0f;
	f3Up.y = 1.0f;
	f3Up.z = 0.0f;

	//set position vector values
	f3Position.x = m_fPositionX;
	f3Position.y = m_fPositionY;
	f3Position.z = m_fPositionZ;

	//set camera view (default) Z
	f3LookAt.x = 0.0f;
	f3LookAt.y = 0.0f;
	f3LookAt.z = 1.0f;

	//Create UP vector
	vUp = XMLoadFloat3(&f3Up);

	//Create Position Vector
	vPosition = XMLoadFloat3(&f3Position);

	//Create LookAt vector
	vLookAt = XMLoadFloat3(&f3LookAt);

	//Set Pitch yaw and roll
	fPitch	= m_fPositionX * RADVALUE;
	fYaw	= m_fPositionY * RADVALUE;
	fRoll	= m_fPositionZ * RADVALUE;
	
	//Create rotation matrix from pitch, yaw and roll
	rotationMatrix = XMMatrixRotationRollPitchYaw(fPitch, fYaw, fRoll);

	//Transform the Look At vector and Up vector by the rotation matrix so the view is correctly rotated at origing
	vLookAt = XMVector3TransformCoord(vLookAt, rotationMatrix);
	vUp = XMVector3TransformCoord(vUp, rotationMatrix);

	//translate the rotated camera position to the location of the viewer
	vLookAt = XMVectorAdd(vPosition, vLookAt);

	//Create matrix from updated vectors

	m_viewMatrix = XMMatrixLookAtLH(vPosition, vLookAt, vUp);

}