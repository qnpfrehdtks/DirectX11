#pragma once
#include "Vertex.h"
#include "EffectMGR.h" 
#include "LightMGR.h"
#include "GeometryGenerator.h"
#include "SkinnedData.h"

#ifndef SHAPE_H
#define SHAPE_H

using namespace Vertexs;


class cCamera;
class cOutLineMap;
class cShadowMap;
class cBasicShape
{

	// Component
private:
	float m_CurTime;
	sBoneAnimation m_ShapeAnimation;
  
	static UINT m_PtCount;
	UINT m_IdxCount;

	sPtLight m_PtLight;
	XMFLOAT4     m_Color;

	float m_fProgress;
	float m_fEdge;

public:
	cBasicShape(ID3D11Device* device, ID3D11DeviceContext* dc);
	virtual ~cBasicShape();

	// ====================================================
	//                          SET
	// ====================================================
	void SettingWorld();

	void SetNormalBuffer();
	void SetBaiscBuffer(GeometryGenerator::MeshData* shape);
	void SetBaiscBuffer(std::vector<sSkinnedVertex>& vec, std::vector<USHORT>& idxVec );
	void SetSimpleBuffer();
	void SetDissolveBuffer();
	void SetSkullBuffer();


	void SetVB(ID3D11Buffer* vb)               { m_VB = vb; }
	void SetIB(ID3D11Buffer* ib)               { m_IB = ib; }

	void SetTech(ID3DX11EffectTechnique* tech) { m_tech = tech; }
	void SetRSV(ID3D11ShaderResourceView* RSV) { m_TextureSRV = RSV; }
	void SetWorld(const CXMMATRIX& mat)        { XMStoreFloat4x4(&m_World, mat); }
	void SetPos(XMFLOAT3 vec)                  { m_Pos = vec; }
	void SetScale(XMFLOAT3 vec)                { m_Scale = vec; }
	void SetRot(XMFLOAT3 vec)                  { m_Rot = vec; }
	void SetView(const CXMMATRIX& mat)         { XMStoreFloat4x4(&m_World, mat); }
	void SetStride(UINT stride)                { m_Stride = stride; }

	void SetOutLine(cOutLineMap* outLine)      { m_outLineMap = outLine; }

	// ====================================================
	//                          GET
	// ====================================================
	ID3D11Buffer* GetVertexBuffer() { return m_VB; }
	ID3D11Buffer* GetIndexBuffer() { return m_IB; }

	ID3D11ShaderResourceView* GetRSV(ID3D11ShaderResourceView* RSV) { return m_TextureSRV; }
	XMFLOAT4X4 GetWorld() { return m_World; }
	XMFLOAT4X4 GetPos() { return m_World; }
	XMFLOAT4X4 GetRot(const CXMMATRIX& mat) { return m_RotMat; }
	XMFLOAT4X4 GetView(const CXMMATRIX& mat) { return m_ScaleMat; }

	UINT GetIdxCount() { return m_IndexCount;  }
	UINT GetVertexCount() { return m_VertexCount; }
	// ====================================================
	//                         ETC
	// ====================================================
	virtual void GbufferDraw(cShadowMap* pShadowMap, cOutLineMap* pOutLineMap);
	virtual void Draw(cShadowMap* pShadowMap, cOutLineMap* pOutLineMap);
	virtual void DissolveDraw(cShadowMap* pShadowMap, cOutLineMap* pOutLineMap,
		ID3D11ShaderResourceView* DissolveSRV, ID3D11ShaderResourceView* DissolveRampSRV);

	void InitAnimation();
	void UpdateDissolve();

	// ====================================================
	//                      Update
	// ====================================================
	void Update(float dt);


protected:
	ID3D11Device * m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;


	// ==============================
	//      world  view  proj Tex
	// ==============================
	XMFLOAT3   m_Pos;
	XMFLOAT3   m_Scale;
	XMFLOAT3   m_Rot;

	XMFLOAT4X4 m_World;
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;
	XMFLOAT4X4 m_Tex;

	XMFLOAT4X4 m_ScaleMat;
	XMFLOAT4X4 m_RotMat;

	// ===============================
	//           Vertex , IDX
	// ===============================
	UINT m_strideSize;
	UINT m_VertexCount;
	UINT m_IndexCount;

	UINT m_VertexOffset;
	UINT m_IdxOffset;

	UINT m_Stride;

	// =================================
	//   Texture or Shader or Material
	// =================================
	sMaterial m_Material;
	cOutLineMap* m_outLineMap;
	XMFLOAT4 m_RimLightColor;

	ID3DX11EffectTechnique* m_tech;
	ID3D11ShaderResourceView* m_TextureSRV;
	ID3D11ShaderResourceView* m_ShadowView;
	ID3D11ShaderResourceView* m_SSAOView;

	XMFLOAT4X4 m_TexTransform;

	BOOL m_isRimLight;
};

class cSimpleShape : public cBasicShape
{

public:
	cSimpleShape(ID3D11Device* device, ID3D11DeviceContext* dc);
	virtual ~cSimpleShape();


	virtual void Draw();


};

class cNormalShape : public cBasicShape
{

public:
	cNormalShape(ID3D11Device* device, ID3D11DeviceContext* dc);
	virtual ~cNormalShape();


	virtual void Draw();


};









#endif
