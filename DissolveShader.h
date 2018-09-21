#pragma once
#include "DXUT.h"
#include <DirectXMath.h>

using namespace DirectX;


// ���� ����. Unity Shader �� DirectX

class DissolveShader
{

private:
	ID3D11ShaderResourceView * m_pOriginTex;
	ID3D11ShaderResourceView * m_pDissolveTex;

	float m_fEdgeAround;
	float m_fEdgeAroundPower;
	float m_fEdgeAroundHDR;
	float m_fEdgeDistortion;

public:
	DissolveShader();
	~DissolveShader();
};

