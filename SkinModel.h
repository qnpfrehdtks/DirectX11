#pragma once
#include "DXUT.h"
#include "SkinData.h"
#include "LightMGR.h"
#include "TextureResourceMGR.h"
#include "SkinnedAnimationData.h"
#include <vector>
#include  "d3dx11effect.h"
#include "AnimationMGR.h"
class cShadowMap;
class  cPointLightShadowMap;
class cOutLineMap;


class cSkinModel
{

private:
	AnimationMGR * m_AniMGR;
	cSkinnedAnimation m_SkinAni;

	// Texture Á¤º¸ //
	std::vector<sMaterial> m_MatArr;
	std::vector<ID3D11ShaderResourceView*> m_DiffuseSRVArr;
	std::vector<ID3D11ShaderResourceView*> m_NormalSRVArr;
	std::vector<ID3D11ShaderResourceView*> m_SpecularSRVArr;

	ID3D11ShaderResourceView* m_NormalMapGolem;
	ID3D11ShaderResourceView* m_HeightMap;

	std::vector<sSkinMesh> m_Meshes;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDC;

	UINT m_MeshCount;

public:
	cSkinModel();
	~cSkinModel();

	//========================================
	//                  ETC
	// =======================================
	void LoadFBXFile(ID3D11Device* device, ID3D11DeviceContext* dc, std::string filename, std::string name, BOOL hasAni);
	void LoadTexture();

	void Draw(ID3DX11EffectTechnique* tech, int PassIdx, UINT stride);
	void GbufferDraw(ID3DX11EffectTechnique* tech, int PassIdx, UINT stride);
	void GbufferDraw2(ID3DX11EffectTechnique* tech, int PassIdx, UINT stride);
	//void POMGbufferDraw(ID3DX11EffectTechnique* tech, int PassIdx, UINT stride);

	void SettingTexture(UINT idx);

	void BoneTransform(float* Time, std::vector<XMFLOAT4X4>& FinalTransform/*, sAnimationClip& aniClip*/);

	//========================================
	//                  GET
	// =======================================
	UINT           GetMeshCount()                            { return m_MeshCount; }
	std::string    GetFirstAniStr()                          { return (*m_SkinAni.GetAniTable().begin()).first; }
	sSkinMesh&     GetMesh(UINT idx)                         { return m_Meshes[idx]; }
	sMaterial&     GetMaterial(UINT idx)                     { return m_MatArr[idx]; }

	ID3D11Buffer*  GetVertexBuffer(UINT idx)                 { return m_Meshes[idx].GetpVertexBuffer();  }
	ID3D11Buffer*  GetIndiceBuffer(UINT idx)                 { return m_Meshes[idx].GetpIndiceBuffer(); }

	UINT           GetIndiceBufferCount(UINT idx)           { return m_Meshes[idx].GetIndices().size(); }

	// get SRVs // 
	ID3D11ShaderResourceView* GetMeshDiffuseMap(UINT idx)   { return m_DiffuseSRVArr[idx]; }
	ID3D11ShaderResourceView* GetMeshNormalMap(UINT idx)    { return m_NormalSRVArr[idx]; }
	ID3D11ShaderResourceView* GetMeshSpecularMap(UINT idx)  { return m_SpecularSRVArr[idx]; }

	AnimationMGR* GetAniMGR() { return m_AniMGR; }

	float GetDuration() { return m_AniMGR->GetDuration(); }
	float GetTimeInTicks() { return m_AniMGR->GetTimeInTicks(); }
	float GetTicksPerSecond() { return m_AniMGR->GetTicksPerSecond(); }

	void SetTimeInTicks(float time) { m_AniMGR->SetTime(time); }


	//========================================
	//                  SET
	// =======================================
	void SetSkinAnimation(cSkinnedAnimation& ani)            { m_SkinAni = ani; }

};

struct sSkinModellInstance
{

	LPSTR TechName;
	cSkinModel* Model;

	BOOL hasAnimation;
	bool isStop;

	float Duration;
	float TicksPerSecond;
	float TimePos;
	float ResultTimePos;
	float AnimationSpeed;

	float ToonTickness;

	std::string ClipName;
	XMFLOAT4X4 World;
	std::vector<XMFLOAT4X4> FinalTransforms;
	UINT m_SubMeshCount;

	//================================================
	//              RimLight Variables
	//================================================
	XMFLOAT4 m_fRimLightColor;
	float m_fRimLightWidth;

	//================================================
	//              Dissolve Variables
	//================================================
	float m_fDissolveProgress;
	float m_fEdge;
	float m_fEdgeRange;

	ID3D11ShaderResourceView* m_pDissolveSRV;
	ID3D11ShaderResourceView* m_pDissolveRampSRV;
	ID3D11ShaderResourceView* m_pBrickHeightMap;

	float GetDuration() { return Model->GetDuration(); }
	float GetTimeInTicks() { return Model->GetTimeInTicks(); }
	float GetTicksPerSecond() { return Model->GetTicksPerSecond(); }
	void SetTimeInTicks(float Time) { Model->SetTimeInTicks(Time); }
	// ===================== Function =======================
	void Update(float dt);
	void Draw(ID3D11DeviceContext* pDC, cShadowMap* shadowMap, cOutLineMap* pOutLineMap);
	void SkinnedGBufferDraw( ID3D11DeviceContext* pDC, cShadowMap* shadowMap, cPointLightShadowMap* ptShadowMap, cOutLineMap* pOutLineMap, bool isPCF);
	void GBufferDraw(ID3D11DeviceContext* pDC, cShadowMap* shadowMap, cOutLineMap* pOutLineMap, CXMMATRIX vp, float heightScale);
	//void POMGbufferDraw(ID3D11DeviceContext* pDC, cShadowMap* shadowMap, cOutLineMap* pOutLineMap, CXMMATRIX vp);

};

