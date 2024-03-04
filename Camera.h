#pragma once

#include <DirectXMath.h>
#include <d3d11_1.h>
using namespace DirectX;

class Camera
{
private:
	//eye at up float 3
	XMFLOAT3 _eye;	// Position
	XMFLOAT3 _at;	// Object to look at
	XMFLOAT3 _up;	// The Up direction

	//Variables to store the window information
	float _windowWidth;
	float _windowHeight;
	float _nearDepth;
	float _farDepth;

	//Camera movement variables
	float _forwardAngle;
	float _rightAngle;
	float _upAngle;
	float _leftRotateAngle;
	float _rightRotateAngle;

	int _reduceSpeed;

	//store view and projection values
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	XMVECTOR Eye;
	XMVECTOR At;
	XMVECTOR Up;

public:
	Camera();
	//constructor function to pass value in to camera
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, float windowWidth, 
			float windowHeight, float nearDepth, float farDepth);

	~Camera();

	//update function
	void Update();

	XMFLOAT4X4 GetView() { return _view; }				//Return view
	XMFLOAT4X4 GetProjection() { return _projection; }	//Return Projection

	//Getter function for Eye, At & Up
	XMFLOAT3 GetEye() { return _eye; }
	XMFLOAT3 GetAt() { return _at; }
	XMFLOAT3 GetUp() { return _up; }

	//Setter function for Eye, At & Up
	void SetEye(XMFLOAT3 eye) { _eye = eye; }
	void SetAt(XMFLOAT3 at) { _at = at; }
	void SetUp(XMFLOAT3 up) { _up = up; }

	//Camera Movement function
	void MoveCamera();
	void RotateCamera();
};

