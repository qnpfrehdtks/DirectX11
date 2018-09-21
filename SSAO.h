#pragma once
#include "DXUT.h"

using namespace DirectX;

class cCamera;

class cSSAO
{

private:

	struct sRandomVector
	{
		FLOAT r;
		FLOAT g;
		FLOAT b;
		FLOAT w;

		sRandomVector()
		{};

	} ;

private:
	float m_fRTVWidth;
	float m_fRTVHeight;

	ID3D11Buffer* m_pVB;
	ID3D11Buffer* m_pIB;

	// SSAO Sampling을 위한 Point들 좌표 입력.
	XMFLOAT4 m_Offsets[14];

	// View Space Pos를 구하기 위해, 절두체 각 코너의 벡터를 입력한다.
	XMFLOAT4 m_Frustum[4];


	ID3D11UnorderedAccessView* m_pSSAOUAVMap01;
	ID3D11UnorderedAccessView* m_pSSAOUAVMap02;


	// SSAO 를 담기 위한 RTV와 Blur를 담기 위해
	ID3D11ShaderResourceView* m_pSSAOMap01;
	ID3D11ShaderResourceView* m_pSSAOMap02;

	// 좀더 좋은 Sample 결과를 위해 Random 벡터를 집어넣음.
	ID3D11ShaderResourceView* m_pRandomVectorSRV;

	
	ID3D11RenderTargetView* m_pRTV01;
	ID3D11RenderTargetView* m_pRTV02;

	D3D11_VIEWPORT m_SSAOViewPort;

	float m_TexWidth;
	float m_TexHeight;

	UINT m_ThreadX;
	UINT m_ThreadY;

	float A;
	float B;

public:
	cSSAO();
	~cSSAO();

	// INIT //
	void Init(float Width, float Height, ID3D11Device* pDevice, ID3D11DeviceContext* pDC, cCamera* cam);
	void BuildTexture(float Width, float Height, ID3D11Device* pDevice, ID3D11DeviceContext* pDC);

	void BuildRandomVectorTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pDC);
	void BuildOffsetVector(ID3D11Device* pDevice, ID3D11DeviceContext* pDC);

	void CreateSSAOMap(ID3D11DeviceContext* pDC, cCamera* camera, ID3D11ShaderResourceView* depthSRV, ID3D11ShaderResourceView* normalSRV, float Radius, float Intensity);
	void BlurSSAOMap(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, cCamera * camera, bool isVert,
		ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,  ID3D11ShaderResourceView* depthSRV, ID3D11ShaderResourceView* normalSRV);

	void BuildTextureCS(float Width, float Height, ID3D11Device* pDevice, ID3D11DeviceContext* pDC);

	void BlurSSAOMapCS(
		ID3D11Device * pDevice,
		ID3D11DeviceContext * pDC, 
		ID3D11ShaderResourceView * depthSRV, 
		ID3D11ShaderResourceView * normalSRV);

	void Blur(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, cCamera * camera,
		UINT blurCount, ID3D11ShaderResourceView* depthSRV, ID3D11ShaderResourceView* normalSRV);


	// SET//
	void SetFrustum(float fov, float farZ);
	void SetScreenQuad(ID3D11Device* pDevice);

	// GET //
	ID3D11ShaderResourceView* GetSSAOMap() { return m_pSSAOMap01; }
	

};

