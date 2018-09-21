#pragma once
#include "DXUT.h"

#ifndef VERTEX_H
#define VERTEX_H

using namespace DirectX;


namespace Vertexs
{

	struct sPos
	{
		XMFLOAT3 Pos;
	};

	struct sSimple
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};


	struct sBasicVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	struct sDissolveVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
		XMFLOAT2 Tex;
	};

	struct sNormalVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 TangentU;
	};

	struct sSkinnedVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 TangentU;
		float Weights[3] = { 0,0,0 };
		int BoneIndices[4] = { 0,0,0,0 };
	};

}

class InputLayOutDesc
{
public:
	static D3D11_INPUT_ELEMENT_DESC PosDesc[1];
	static D3D11_INPUT_ELEMENT_DESC SimpleDesc[2];
	static D3D11_INPUT_ELEMENT_DESC BasicDesc[3];
	static D3D11_INPUT_ELEMENT_DESC DissolveDesc[3];
	static D3D11_INPUT_ELEMENT_DESC NormalBasicDesc[4];
	static D3D11_INPUT_ELEMENT_DESC SkinDesc[6];

public:
	InputLayOutDesc() {};
	~InputLayOutDesc()
	{

	}
};

class InputLayOut
{
public:

	static void InitAllInputLayout(ID3D11Device* device);

	static ID3D11InputLayout* PosInputLayout;
	static ID3D11InputLayout* SimpleInputLayout;
	static ID3D11InputLayout* BasicInputLayout;
	static ID3D11InputLayout* DissolveInputLayout;
	static ID3D11InputLayout* NormalInputLayout;
	static ID3D11InputLayout* SkinInputLayout;

public:
	InputLayOut() {}
	~InputLayOut()
	{

	}
};
//
//#pragma region InputLayoutDesc
//
//D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::PosDesc[1] =
//{
//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//};
//
//D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::BasicDesc[3] =
//{
//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//};
//
//D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::NormalBasicDesc[4] =
//{
//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
//{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//};
//
//#pragma endregion
//
//#pragma region inputLayout
//ID3D11InputLayout* InputLayOut::BasicInputLayout = 0;
//ID3D11InputLayout* InputLayOut::NormalInputLayout = 0;
//ID3D11InputLayout* InputLayOut::PosInputLayout = 0;
//#pragma endregion

#endif
