#include "GameObject.h"


GameObject::GameObject() {}

void GameObject::Update()
{
	XMStoreFloat4x4(&_world, XMLoadFloat4x4(&_scale) * XMLoadFloat4x4(&_rotate) * XMLoadFloat4x4(&_translate));		//Apply all Transform value on the object
}

void GameObject::SetTranslation(XMFLOAT3 position)
{
	XMStoreFloat4x4(&_translate, XMMatrixTranslation(position.x, position.y, position.z));		//Set Translate of the object
}

void GameObject::SetRotation(XMFLOAT3 eulerAngles)
{
	XMVECTOR eulerVec = XMLoadFloat3(&eulerAngles);

	XMStoreFloat4x4(&_rotate, XMMatrixRotationRollPitchYawFromVector(eulerVec));				// Set Rotation of the object
}

void GameObject::SetScale(XMFLOAT3 scale)
{
	XMStoreFloat4x4(&_scale, XMMatrixScaling(scale.x, scale.y, scale.z));						// Set Scale of the object
}