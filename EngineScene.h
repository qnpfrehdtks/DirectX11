#pragma once
#include "BaseScene.h"
#include "DXUTcamera.h"
#include "LightMGR.h"
#include "SDKmesh.h"
#include "BasicModel.h"
#include "SkinModel.h"
#include "SkinnedModel.h"
#include "Gbuffer.h"
#include "DeferredShading.h"
#include "SSAO.h"
#include "TileDeferredingShading.h"
#include "Glow.h"
#include "HeatHaze.h"
#include "FlameEffect.h"
#include "SSR.h"
#include "PointLightShadow.h"
//#include "FbxSkinModel.h"


class cBlurFilter;
class cBasicShape;
class cSimpleShape;
class cOutLineMap;
class cHDR;
class cBloomFilter;
class cOffScreenTextureView;

enum RenderingMode
{
	FORWARDING_RENDERING,
	DEFERRED_SHADING,
	TILE_DEFERRED_SHADING
};

enum RenderMode
{
	NORMAL,
	DIFFUSE,
	TILE,
	DEPTH,
	SSAO

};

using namespace DirectX;

class cEngineScene : public cBaseScene
{

private:
	sSkinModellInstance m_SponzaChaIns;
	sSkinModellInstance m_GolemChaIns;
	sSkinnedModelInstance m_ChaIns;
	cSkinnedModel * m_Cha;
	cSkinModel* m_GolemCha;
	cSkinModel* m_Sponza;
	cGlow m_Glow;
	cHeatHaze m_HeatHaze[4];
	cFlameEffect m_FlameEffect;
	cGBuffer m_Gbuffer;
	cPointLightShadowMap m_PtShadowMap;
	
	cSSR m_SSR;

	float angle = 0;
	float heightScale = 0.01f;
	float SSRFactor = 0;

	std::vector<BasicModelInstance*> m_ModelVec;
	BasicModelInstance* m_Tree;


	ID3D11ShaderResourceView * m_DissolveRampSRV;
	ID3D11ShaderResourceView * m_DissolveSRV;
	ID3D11ShaderResourceView* m_LandSRV;
//	cOutLineMap* m_OutLineMap;
	cBlurFilter* m_BlurFilter;
	cHDR*        m_PostFX;
	cBloomFilter* m_BloomFilter;
	cOffScreenTextureView* m_OffSceenView;
	DeferredShading m_DeferredShading;
	TileDeferredingShading m_TileDeferredShading;
	cSSAO m_ssao;


	XMFLOAT4X4 m_world;

	sMaterial Mat;

	/*ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;*/

	XMFLOAT4X4 m_SphereWorld;

	ID3D11Buffer* m_SphereVB;
	ID3D11Buffer* m_SphereIB;

	std::vector<cBasicShape*> m_Shapes;

	//CFirstPersonCamera m_Cam;
	UINT m_VertexCount;
	UINT m_IndexCount;
	UINT m_SphereCount;

	// Scene meshs
	CDXUTSDKMesh m_MeshOpaque;
	CDXUTSDKMesh m_MeshSphere;


	LPCSTR m_TechName;

	cCamera* m_pCam;

	bool m_isSSAO;
	bool m_isSSAOBlur;
	bool m_isShadowPCF;

	float m_SSAOIntensity;
	float m_SSAORadius;
	float m_GlowSpeed;
	float m_Lightcolor[4];
	float m_DiffuseLightcolor[4];
	float m_SpecLightcolor[4];
	float m_LightDirAngle[3];

	RenderingMode m_RenderingMode;
	int m_RenderMode;


public:
	cEngineScene(ID3D11Device* device, ID3D11DeviceContext* dc, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv,
		 ID3D11ShaderResourceView* srv);
	~cEngineScene();

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void ForwardLighting();
	void DeferredLighting();
	void TileDeferredLighting();

	void DrawSceneToShadowMap();

	void UIControll();
	void ForwardingUIControll();
//	void DrawSceneToOutLineMap();
//	void DrawSceneToBlurMap();

	// Blur//

//	void Blur(int BlurCount);
	//void Blur(ID3D11ShaderResourceView* rsv, ID3D11RenderTargetView* outputRTV, bool horzBlur);




};

