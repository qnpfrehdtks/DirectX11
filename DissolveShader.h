#pragma once
#include "DXUT.h"
#include <DirectXMath.h>

using namespace DirectX;


// 원리 참고. Unity Shader 를 DirectX

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

