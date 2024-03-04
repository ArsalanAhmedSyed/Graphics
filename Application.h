#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "Structure.h"
#include "OBJLoader.h"
#include "Camera.h"
#include "GameObject.h"

using namespace DirectX;



class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pPyramidVertexBuffer;
	ID3D11Buffer*           _pSkyboxVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pPyramidIndexBuffer;
	ID3D11Buffer*           _pSkyboxIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world,_world4[5];
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;
	ID3D11RasterizerState*  _wireFrame;
	ID3D11RasterizerState*	_solidState;

	//Depth
	ID3D11Texture2D*		_depthStancilBuffer;
	ID3D11DepthStencilView* _depthStancilView; //viewing or rendering what is on the screen and not what is not

	//Lighting 
	XMFLOAT3 directionToLight;

	XMFLOAT4 DiffuseLight;
	XMFLOAT4 DiffuseMatieral;

	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;

	XMFLOAT4 SpecularMat;
	XMFLOAT4 SpecularLight;
	FLOAT SpecularPower;		//Power to raise specular falloff by
	XMFLOAT3 EyeWorldPos;		//Cameras Eye Position in the world

	//Fog
	float fogStart, fogEnd;

	//Boolean Check
	UINT enableSkyBox;
	UINT fogEnable;
	//UINT crateTextureEnabe;

	//Texture
	ID3D11ShaderResourceView* _pTextureRV = nullptr;		//Crate Texture Holder
	ID3D11ShaderResourceView* _pSkyboxTexture = nullptr;	//Skybox Texture Holder
	ID3D11SamplerState* _pSamplerLinear = nullptr;			//Texture Sampler
	ID3D11SamplerState* _pSkyboxSamplerLinear = nullptr;			//Texture Sampler

	//Object
	MeshData myObjectMeshData;

	//Camera Access
	Camera _camera1, _camera2, _camera3;
	Camera _currentCamera;

	//GameObjects
	GameObject _cubeObject, _cubeObject2; //CubeObject
	MeshData* pCubeData;
	//PyramidObject
	GameObject _pyramidObject;
	MeshData* pPyramidData;

	//Skybox Setup
	GameObject _skybox;
	MeshData* pSkyboxData;

	//Blending
	ID3D11BlendState* Transparency;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};