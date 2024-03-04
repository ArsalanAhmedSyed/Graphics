#include "Camera.h"

Camera::Camera() {}

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth)
{
	//Passing values from paramiter 
	_eye = position;
	_at = at;
	_up = up;

	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;

	//Initialize the view matrix
	Eye = XMLoadFloat3(&_eye);
	At = XMLoadFloat3(&_at);
	Up = XMLoadFloat3(&_up);
	XMStoreFloat4x4(&_view, XMMatrixLookToLH(Eye, At, Up));	
	
	//Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));

	//
	//Initialize the moving camera variables
	//
	
	//Direction set
	_forwardAngle = 1;
	_rightAngle = 0;
	_upAngle = 0;

	//Rotation Set
	_leftRotateAngle = 0;
	_rightRotateAngle = 0;

	//Direction Speed set
	_reduceSpeed = 100;
}

Camera::~Camera() {}

void Camera::Update()
{
	MoveCamera();
	RotateCamera();

	//Update view matrix
	Eye = XMLoadFloat3(&_eye);
	At = XMLoadFloat3(&_at);
	Up = XMLoadFloat3(&_up);
	XMStoreFloat4x4(&_view, XMMatrixLookToLH(Eye, At, Up));
}

void Camera::MoveCamera()
{
	if (GetAsyncKeyState(VK_UP) < 0){			// Up Arrow Key
		_eye.z += 0.1 / _reduceSpeed;			// Move Forward
	}
	else if (GetAsyncKeyState(VK_DOWN) < 0){	// Down Arrow Key
		_eye.z -= 0.1 / _reduceSpeed;			// Move Backward
	}

	if (GetAsyncKeyState(VK_LEFT) < 0){			// Left Arrow Key
		_eye.x -= 0.1 / _reduceSpeed;			// Move Left
	}
	else if (GetAsyncKeyState(VK_RIGHT) < 0){	// Right Arrrow Key
		_eye.x += 0.1 / _reduceSpeed;			// Move Right
	}

	if (GetAsyncKeyState(VK_F1) < 0){			// F1 Key
		_eye.y += 0.1 / _reduceSpeed;			// Move Up
	}
	else if (GetAsyncKeyState(VK_F2) < 0){		// F2 Key
		_eye.y -= 0.1 / _reduceSpeed;			// Move Down
	}
}

void Camera::RotateCamera()
{
	if (GetAsyncKeyState(VK_F3) < 0)			// F3 Key
	{
		_at.x -= 0.1 / 1000;					//Rotate to Left
		_at.z -= 0.1 / 1000;
	}

	if (GetAsyncKeyState(VK_F4) < 0)			// F4 Key
	{
		_at.x += 0.1 / 1000;					//Rotate to Right
		_at.z += 0.1 / 1000;
	}
}