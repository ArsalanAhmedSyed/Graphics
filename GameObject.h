#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include "Structure.h"

using namespace DirectX;

class GameObject
{
private:
	MeshData _meshData;

	//Transform Value
	XMFLOAT4X4 _translate;
	XMFLOAT4X4 _rotate;
	XMFLOAT4X4 _scale;

	XMFLOAT4X4 _world;

public:
	GameObject();

	void Update();							//Updaing the transform
	void SetTranslation(XMFLOAT3 position); //set positon
	void SetRotation(XMFLOAT3 eulerAngles); //Set Rotation
	void SetScale(XMFLOAT3 scale);			//Set Scale

	MeshData* GetMeshData() { return &_meshData; }			//Get Meshdata

	void SetWorld(XMFLOAT4X4 world) { _world = world; }		//Set world
	XMFLOAT4X4* GetWorld() { return &_world; }				//Get world

};

