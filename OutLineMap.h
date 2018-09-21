#pragma once
#include "Main.h"
#include"NormalDepthMap.h"

using namespace DirectX;


class cSimpleShape;
class cBasicShape;


class cOutLineMap
{
public:
	cOutLineMap(ID3D11Device* device, ID3D11DeviceContext* dc, float width, float height);
	~cOutLineMap();

	
public:

	void Init();
	void OnSize();
	void BuildScreenQuad();


	// ===============================================
	//                       GET
	// ===============================================
	ID3D11RenderTargetView*    GetRTV()              { return m_SilhouetteEdgeRTV; }
	ID3D11ShaderResourceView * GetOutLineSRVMap()    { return m_SilhouetteEdgeSRV; }
	XMFLOAT4X4                 GetOutLineTransform() { return m_OutLineTransform; }
	ID3D11Buffer*              GetBufferVB()         { m_SrcreenQuadVB; }
	ID3D11Buffer*              GetBufferIB()         { m_SrcreenQuadIB; }

	cNormalDepthMap* GetNormalDepthMap()             { return m_NormalDepthMap; }


private:
	ID3D11Device * m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	XMFLOAT4X4           m_OutLineTransform;
	XMFLOAT4             m_Offset[14];

	ID3D11ShaderResourceView * m_SilhouetteEdgeSRV;
	ID3D11RenderTargetView *   m_SilhouetteEdgeRTV;

	D3D11_VIEWPORT m_ViewPort;

	ID3D11Buffer* m_SrcreenQuadIB;
	ID3D11Buffer* m_SrcreenQuadVB;


	UINT m_VBSize;
	UINT m_IBSize;

	cNormalDepthMap* m_NormalDepthMap;

	float m_height;
	float m_width;

private:
	void CreateOutLineTexture(ID3D11Device* device, ID3D11DeviceContext* dc);
	void BuildOutLineTransform();
	void BuildOffsetVector();

	

public:
	void Update(float dt);
	// NormalMap에 Normal Depth 를 새겼으니 ,이제 이 Normal map을 통해 
	// 외곽 라인을 칠하자.
	void DrawToNormalDepthMap(cCamera* cam);
	void ComputeOutLineMap(cCamera* cam);

	void InsertShape(cBasicShape* shape);

};

