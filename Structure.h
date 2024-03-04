#pragma once
#include <DirectXMath.h>
#include <d3d11_1.h>
using namespace DirectX;

struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TextureCoord;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	//Diffuse 
	XMFLOAT4 DiffLight;
	XMFLOAT4 DiffMat;
	XMFLOAT3 DireToLight;

	float padding;
	//Ambient 
	XMFLOAT4 AmbLight;
	XMFLOAT4 AmbMat;

	//Specular
	XMFLOAT4 SpecMat;
	XMFLOAT4 SpecLight;
	float SpecPower;
	XMFLOAT3 EyeWorldPos;

	//Fog
	float startFog;
	float endFog;

	//Enable feature
	UINT enableFog;
	UINT skyBoxEnable;
};
