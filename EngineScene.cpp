#include "DXUT.h"
#include "EngineScene.h"
#include "EffectMGR.h"
#include "GeometryGenerator.h"
#include "Main.h"
#include "cShape.h"
#include "Sky.h"
#include "MathHelper.h"
#include "LightMGR.h"
#include "ShadowMap.h"
#include "OutLineMap.h"
#include "RenderState.h"
#include "BlurFilter.h"
#include "BloomFilter.h"
#include "HDR.h"
#include "OffScreenTextureView.h"
#include "FBXLoader.h"
#include <DirectXTex.h>

#include "WICTextureLoader.h"
#include <iostream>
#include <string>
#include <fstream>      // std::ifstream

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "Camera.h"

#include "FBXLoader.h"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

#include <fbxsdk.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"



using namespace Vertexs;

const char* UIChar[] = { "All", "Normal", "SSAO", "Tile", "Depth", "Diffuse" };

cEngineScene::cEngineScene
(ID3D11Device* device,
	ID3D11DeviceContext* dc,
	ID3D11RenderTargetView* rtv,
	ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView* srv)
	: cBaseScene(device, dc, rtv, dsv), m_LandSRV(0), m_TechName("TileSSAO"),
	m_isSSAO(true), m_RenderingMode(TILE_DEFERRED_SHADING), SSRFactor(0.0f),
	m_SSAOIntensity(2.0f), m_SSAORadius(3.0f), m_GlowSpeed(0.05f), m_RenderMode(0)
{

	FbxManager* g_pFBXManager = nullptr;
	// World 좌표 설정.
	XMStoreFloat4x4(&m_world, XMMatrixIdentity());

	LightMGR::GetInstance()->Init();
	m_Lightcolor[0] = LightMGR::GetInstance()->GetDirLight(0).Ambient.x;
	m_Lightcolor[1] = LightMGR::GetInstance()->GetDirLight(0).Ambient.y;
	m_Lightcolor[2] = LightMGR::GetInstance()->GetDirLight(0).Ambient.z;
	m_Lightcolor[3] = 1.0f;

	m_DiffuseLightcolor[0] = LightMGR::GetInstance()->GetDirLight(0).Diffuse.x;
	m_DiffuseLightcolor[1] = LightMGR::GetInstance()->GetDirLight(0).Diffuse.y;
	m_DiffuseLightcolor[2] = LightMGR::GetInstance()->GetDirLight(0).Diffuse.z;
	m_DiffuseLightcolor[3] = 1.0f;

	m_SpecLightcolor[0] = LightMGR::GetInstance()->GetDirLight(0).Specular.x;
	m_SpecLightcolor[1] = LightMGR::GetInstance()->GetDirLight(0).Specular.y;
	m_SpecLightcolor[2] = LightMGR::GetInstance()->GetDirLight(0).Specular.z;
	m_SpecLightcolor[3] = 1.0f;


	m_LightDirAngle[0] = LightMGR::GetInstance()->GetDirLight(0).Direction.x;
	m_LightDirAngle[1] = LightMGR::GetInstance()->GetDirLight(0).Direction.y;
	m_LightDirAngle[2] = LightMGR::GetInstance()->GetDirLight(0).Direction.z;
//


	// 평행광 초기화.
	
	/*Mat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	Mat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 12.0f);
	Mat.Reflect = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);*/


	const aiScene* walkingMan;

}

cEngineScene::~cEngineScene()
{
}

bool cEngineScene::Init()
{
	// Texture 불러오기.
	m_LandSRV = TextureResourceMGR::GetInstance()->CreateSRV(m_pDevice, L"Textures/ice.dds");
	m_DissolveRampSRV = TextureResourceMGR::GetInstance()->CreateSRVFromWIC(m_pDevice, L"Textures/tex_gradient.png");
	m_DissolveSRV = TextureResourceMGR::GetInstance()->CreateSRVFromWIC(m_pDevice, L"Textures/tex_dissolve.png");

	// Effect 초기화.
	RenderStates::InitAll(m_pDevice);
	EffectMGR::InitAll(m_pDevice);

	// 입력 LayOut 초기화. 
	InputLayOut::InitAllInputLayout(m_pDevice);

	m_pCam = D3DMain::GetInstance()->GetCam();
	m_Sky = new cSky(m_pDevice, L"Textures/grasscube1024.dds", 10.0f);


	// ==========================================================================================
	// Gbuffer 초기화...// 
	m_Gbuffer.Init(m_pDevice, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	m_DeferredShading.Init(m_pDevice, m_pDC);

	// ==========================================================================================
	// SSAO 초기화...// 
	m_ssao.Init(D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight(), m_pDevice, m_pDC, m_pCam);
	
	// ==========================================================================================

	m_BlurFilter = new cBlurFilter(m_pDevice, m_pDC, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	m_BlurFilter->BuildOffScreenGeometry();

	m_ShadowMap = new cShadowMap( m_pDevice, m_pDC, 2048 , 2048 );
	m_ShadowMap->Init();

	// boblampclean.md5mesh
	// Skull //
	m_Sponza = new cSkinModel();
	m_Sponza->LoadFBXFile(m_pDevice, m_pDC, "Models/sponza3.FBX", "sponza", false);
	m_SponzaChaIns.Model = m_Sponza;
	m_SponzaChaIns.TimePos = 0.0f;
	m_SponzaChaIns.m_SubMeshCount = m_Sponza->GetMeshCount();
	XMStoreFloat4x4(&m_SponzaChaIns.World, XMMatrixRotationX(XMConvertToRadians(90.0f)) * XMMatrixScaling(0.2f, 0.2f, 0.2f));


	m_GolemCha = new cSkinModel();
	m_GolemCha->LoadFBXFile(m_pDevice, m_pDC, "Models/BlackGolem.FBX", "SkullDragon", true);
	m_GolemChaIns.Model = m_GolemCha;
	m_GolemChaIns.TimePos = 0.0f;
	m_GolemChaIns.AnimationSpeed = 1.0f;
	m_GolemChaIns.m_SubMeshCount = m_GolemCha->GetMeshCount();
	m_GolemChaIns.m_fRimLightColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_GolemChaIns.m_fRimLightWidth = 0.5f;
	m_GolemChaIns.ToonTickness = 1.2f;
	m_GolemChaIns.TechName  = "SkinDirLightBumped";
	m_GolemChaIns.isStop = false;

	m_GolemChaIns.m_fDissolveProgress = 1.0f;
	m_GolemChaIns.m_fEdge = 0.1f;
	m_GolemChaIns.m_fEdgeRange = 0.1f;
	m_GolemChaIns.m_pDissolveRampSRV = m_DissolveRampSRV;
	m_GolemChaIns.m_pDissolveSRV = m_DissolveSRV;


	XMMATRIX modelScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	XMMATRIX modelRot = XMMatrixRotationX(0.0f);
	XMMATRIX  modelOffset = XMMatrixTranslation(103.62f, 0.00f, 15.32f);
	XMStoreFloat4x4(&m_GolemChaIns.World, modelScale * modelRot * modelOffset);

	cBasicShape* shape[800];

	m_SphereCount = 4;

	GeometryGenerator geo;
	GeometryGenerator::MeshData mesh;


	geo.CreateSphere(1.0f, 10, 10, mesh);
	for (int i = 0; i < 796; i++)
	{
		shape[i] = new cBasicShape(m_pDevice, m_pDC);
		delete shape[i];
	}

	

	m_Glow.Init(m_pDevice, m_pDC);
	
	m_HeatHaze[0].Init(m_pDevice, m_pDC, XMFLOAT3(97.8f, 34.5f, 44.518f));
	m_HeatHaze[1].Init(m_pDevice, m_pDC, XMFLOAT3(-124, 34.5f, 44.518f));
	m_HeatHaze[2].Init(m_pDevice, m_pDC, XMFLOAT3(-124, 34.5f, -28.518f));
	m_HeatHaze[3].Init(m_pDevice, m_pDC, XMFLOAT3(97.8f, 34.5f, -28.518f));

	XMFLOAT3 FlamePos[4];
	FlamePos[0] = XMFLOAT3(97.8f, 31.5f, 44.518f);
	FlamePos[1] = XMFLOAT3(-124, 31.5f, 44.518f);
	FlamePos[2] = XMFLOAT3(-124, 31.5f, -28.518f);
	FlamePos[3] = XMFLOAT3(97.8f, 31.5f, -28.518f);

	m_FlameEffect.Init(m_pDevice, m_pDC, 25, FlamePos, sizeof(FlamePos) / sizeof(XMFLOAT3));

	m_TileDeferredShading.Init(m_pDevice, m_pDC);

//	m_PtShadowMap.Init(m_pDevice, m_pDC, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	//m_PtShadowMap.BuildCubeFaceCam(97.8f, 31.5f, 44.518f);

//	m_SSR.Init(m_pDevice, m_pDC, 0, 0);

	m_PostFX = new cHDR(m_pDevice, m_pDC, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	m_PostFX->Init();

	m_BloomFilter = new cBloomFilter(m_pDevice, m_pDC, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	m_BloomFilter->Init();

	m_OffSceenView = new cOffScreenTextureView(m_pDevice, m_pDC, D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight());
	m_OffSceenView->Init();

	return true;
}

void cEngineScene::OnResize()
{
	m_BlurFilter->Init();
	m_pCam->SetProjection(XM_PI * 0.25f, m_pCam->GetAspect(), 0.1f, 3000.0f);
	m_ssao.BuildTexture(D3DMain::GetInstance()->GetWidth(), D3DMain::GetInstance()->GetHeight(), m_pDevice, m_pDC);
	m_ssao.SetFrustum(m_pCam->GetFovY(), m_pCam->GetFarZ());
}

void cEngineScene::UpdateScene(float dt)
{
//	LightMGR::GetInstance()->UpdateLight(dt);
	LightMGR::GetInstance()->UpdatePointLight(dt, m_GlowSpeed);

	m_ShadowMap->UpdateShadow(dt);
	m_GolemChaIns.Update(dt);

	if (m_RenderingMode == TILE_DEFERRED_SHADING) {

		if (m_isSSAO) {
			m_TechName = "TileSSAO";
		}
		else {
			m_TechName = "TileTest";
		}
	}

	m_FlameEffect.Update(dt);

	for (int i = 0; i < 4; i++) {
	  m_HeatHaze[i].Update(dt);
	
    }

	m_TileDeferredShading.LightUpdate(m_pDevice, m_pDC);
	m_Glow.Update(m_pDC);
	

}

void cEngineScene::DrawScene()
{
	switch (m_RenderingMode)
	{
	  case FORWARDING_RENDERING :
		ForwardLighting();
		break;
	  case DEFERRED_SHADING:
		DeferredLighting();
		break;
	  case TILE_DEFERRED_SHADING:
		TileDeferredLighting();
		break;
	}
}

void cEngineScene::ForwardLighting()
{
	//DrawSceneToShadowMap();

	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

#ifndef Sky Rendering
	m_OffSceenView->SwapToOffScreen(m_pDSV);

#endif

	XMMATRIX V = m_pCam->GetViewMat();
	XMMATRIX P = m_pCam->GetProjMat();

	XMMATRIX VP = V * P;

#ifndef Skinned Mesh Rendering

	m_Sky->DrawSky(m_pDC, D3DMain::GetInstance()->GetCam());

	m_GolemChaIns.Draw(m_pDC, m_ShadowMap, nullptr );
#endif

#ifndef Basic Object Rendering
	//for (int i = 0; i < 2; i++)
	//{
	//	m_Shapes[i]->SetRSV(m_LandSRV);
	//	m_Shapes[i]->GbufferDraw(m_ShadowMap, nullptr);
	//}
#endif

#ifndef Blur + HDR Region
	// ================= Swap from a OffScreen RTV to original RTV ===============
	ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
	m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthReadOnlyDSV());
	// ===========================================================================

	// =======================================================
	//                  RTV Clear 해주어 초기화.
	// ==================== Clear RTV ========================
	float color[] = { 0.0f,0.0f,-1.0f,1e5f };
	m_pDC->ClearRenderTargetView(m_pRTV, color);
	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// =======================================================
	//       OffScreen 에 그려진 기존 이미지를 흐리기 진행. 
	// =======================================================
	m_pDC->OMSetRenderTargets(1, renderTargets, 0);

	// ============================================
	//         깊이, 스텐실 및 뷰포트 복구//

	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	m_BloomFilter->PostProcessing(m_OffSceenView->GetOffSceenSRV(), m_pRTV, m_pDSV);

	ForwardingUIControll();
	ImGui::Render();

	renderTargets[0] = m_pRTV;
	m_pDC->OMSetRenderTargets(1, renderTargets, m_pDSV);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());




#endif
}

void cEngineScene::DeferredLighting()
{
	//DrawSceneToShadowMap();

	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	

	m_Gbuffer.SwapToGbufferRTV(m_pDC);

#ifndef Blur + HDR 미 사용시에, 활성화 할것.

	//m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	//ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
	//m_pDC->OMSetRenderTargets(1, renderTargets, m_pDSV);

	//m_pDC->ClearRenderTargetView(m_pRTV, Colors::Bisque);
	//m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif

#ifndef Sky Rendering
	//m_OffSceenView->SwapToOffScreen(m_pDSV);

#endif

	XMMATRIX V = m_pCam->GetViewMat();
	XMMATRIX P = m_pCam->GetProjMat();

	XMMATRIX VP = V * P;

#ifndef Skinned Mesh Rendering
	//m_GolemChaIns.SkinnedGBufferDraw(m_pDC, m_ShadowMap, nullptr );
	m_SponzaChaIns.GBufferDraw(m_pDC, m_ShadowMap, nullptr, VP,heightScale);
	//m_Glow.DrawGlowMap(m_pDC, m_pCam->GetPos(), VP);

#endif

#ifndef Basic Object Rendering
	//for (int i = 0; i < 2; i++)
	//{
	//	m_Shapes[i]->SetRSV(m_LandSRV);
	//	m_Shapes[i]->GbufferDraw(m_ShadowMap, nullptr);
	//}
#endif
	m_Gbuffer.PostProcessing(m_pDC);


#ifndef Blur + HDR Region
	// ================= Swap from a OffScreen RTV to original RTV ===============
	ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
	m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthReadOnlyDSV());
	// ===========================================================================

	// =======================================================
	//                  RTV Clear 해주어 초기화.
	// ==================== Clear RTV ========================
	float color[] = { 0.0f,0.0f,-1.0f,1e5f };
	m_pDC->ClearRenderTargetView(m_pRTV, color);
	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// =======================================================
	//       OffScreen 에 그려진 기존 이미지를 흐리기 진행. 
	// =======================================================
	m_pDC->OMSetRenderTargets(1, renderTargets, 0);

//	if (m_isSSAO) {
//		m_ssao.CreateSSAOMap(m_pDC, m_pCam, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV());
//		m_ssao.BlurSSAOMapCS(m_pDevice, m_pDC, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV());
//	}

	// ============================================
	//         깊이, 스텐실 및 뷰포트 복구//
	// ============================================
	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	// renderTargets[0] = { nullptr };
	// m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthReadOnlyDSV());
	m_OffSceenView->SwapToOffScreen(m_Gbuffer.GetDepthReadOnlyDSV());

	// ID3D11RenderTargetView* rtv[1] = { nullptr };
	// m_pDC->OMSetRenderTargets(1, rtv, m_Gbuffer.GetDepthDSV());
	m_DeferredShading.LightPass(m_pDC, &m_Gbuffer, m_pCam, "DeferredDirLight", m_ShadowMap, m_ssao.GetSSAOMap());
	// m_TileDeferredShading.LightPass(m_pDC, &m_Gbuffer, D3DMain::GetInstance()->GetCam(), m_TechName, m_ShadowMap, m_ssao.GetSSAOMap(), VP);

	m_BloomFilter->PostProcessing(m_OffSceenView->GetOffSceenSRV(), m_pRTV, m_Gbuffer.GetDepthDSV());

	renderTargets[0] = m_pRTV;
	m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthDSV());
  // m_OffSceenView->DrawToScene(m_TileDeferredShading.GetResultSRV());
	// 
//	m_Sky->DrawSky(m_pDC, D3DMain::GetInstance()->GetCam());
//
//	for (int i = 0; i < 4; i++) {
//		m_HeatHaze[i].DrawHeatHaze(m_pDevice, m_pDC, m_BloomFilter->GetResultSRV(), VP);
//		m_FlameEffect.Draw(m_pDevice, m_pDC, m_pCam, VP);
//	}

#endif
}

void cEngineScene::TileDeferredLighting()
{

 
	DrawSceneToShadowMap();


	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());


	m_Gbuffer.SwapToGbufferRTV(m_pDC);

#ifndef Blur + HDR 미 사용시에, 활성화 할것.

	//m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	//ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
	//m_pDC->OMSetRenderTargets(1, renderTargets, m_pDSV);

	//m_pDC->ClearRenderTargetView(m_pRTV, Colors::Bisque);
	//m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
#endif

#ifndef Sky Rendering
	//m_OffSceenView->SwapToOffScreen(m_pDSV);

#endif
	XMMATRIX V = m_pCam->GetViewMat();
	XMMATRIX P = m_pCam->GetProjMat();

	XMMATRIX VP = V * P;



#ifndef Skinned Mesh Rendering
	m_GolemChaIns.SkinnedGBufferDraw(m_pDC, m_ShadowMap,&m_PtShadowMap, nullptr, m_isShadowPCF);
	m_SponzaChaIns.GBufferDraw(m_pDC, m_ShadowMap, nullptr, VP, heightScale);
	m_Glow.DrawGlowMap(m_pDC, m_pCam->GetPos(), VP);

#endif
	m_Gbuffer.PostProcessing(m_pDC);

#ifndef Blur + HDR Region
	// ================= Swap from a OffScreen RTV to original RTV ===============
	ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
	renderTargets[0] = { nullptr };
	m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthReadOnlyDSV());
	// ===========================================================================


	// =======================================================
	//                  RTV Clear 해주어 초기화.
	// ==================== Clear RTV ========================
	float color[] = { 0.0f,0.0f,-1.0f,1e5f };
	//m_pDC->ClearRenderTargetView(m_pRTV, color);
	//m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (m_isSSAOBlur) {
		m_ssao.CreateSSAOMap(m_pDC, m_pCam, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV(), m_SSAORadius, m_SSAOIntensity);
		m_ssao.BlurSSAOMapCS(m_pDevice, m_pDC, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV());
		//m_ssao.Blur(m_pDevice, m_pDC, m_pCam, 2, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV());
	}
	else
	{
		m_ssao.CreateSSAOMap(m_pDC, m_pCam, m_Gbuffer.GetDepthSRV(), m_Gbuffer.GetNormalSRV(), m_SSAORadius, m_SSAOIntensity);
	}

	// ============================================
	//         깊이, 스텐실 및 뷰포트 복구            
	// ============================================
	m_pDC->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDC->RSSetViewports(1, &D3DMain::GetInstance()->GetViewPort());

	//m_OffSceenView->SwapToOffScreen(m_Gbuffer.GetDepthReadOnlyDSV());

	m_TileDeferredShading.LightPass(m_pDC, &m_Gbuffer, D3DMain::GetInstance()->GetCam(), m_TechName, m_ShadowMap, &m_PtShadowMap, m_ssao.GetSSAOMap(), VP, m_RenderMode);
	m_BloomFilter->PostProcessing(m_TileDeferredShading.GetResultSRV(), m_pRTV, m_pDSV);


	m_pDC->OMSetDepthStencilState(RenderStates::MirrorDSS, 1);

	UIControll();
	ImGui::Render();

	renderTargets[0] = m_pRTV;
	m_pDC->OMSetRenderTargets(1, renderTargets, m_pDSV);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_pDC->OMSetDepthStencilState(0, 0);

	renderTargets[0] = m_pRTV;
	m_pDC->OMSetRenderTargets(1, renderTargets, m_Gbuffer.GetDepthDSV());


	for (int i = 0; i < 4; i++) {
		m_HeatHaze[i].DrawHeatHaze(m_pDevice, m_pDC, m_BloomFilter->GetResultSRV(), VP);
	}

	m_FlameEffect.Draw(m_pDevice, m_pDC, m_pCam, VP);

	

#endif
}

void cEngineScene::DrawSceneToShadowMap()
{
	m_ShadowMap->SwapRenderTargetToShadow();
	cCamera* cam =  D3DMain::GetInstance()->GetCam();
	m_ShadowMap->DrawToShadowMap(m_GolemChaIns, cam,0 , true );

	m_pDC->RSSetState(0);
}

void cEngineScene::UIControll()
{

	bool isOpen = true;
	ImGui::Begin("", &isOpen, ImVec2(450, 320), 0.9f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(620, 10));

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	

	ImGui::Checkbox("Shadow PCF", &m_isShadowPCF);
	ImGui::BeginChild("Render mode", ImVec2(370, 40), true);
	ImGui::Combo("", &(m_RenderMode), UIChar, 6);
	ImGui::EndChild();

	ImGui::Text("SSAO Option");
	ImGui::BeginChild("SSAO Option", ImVec2(370,80), true);

	ImGui::Checkbox("SSAOBlur", &m_isSSAOBlur);
	ImGui::SameLine();
	ImGui::Checkbox("SSAO", &m_isSSAO);
	ImGui::SliderFloat("SSAOIntensity", &m_SSAOIntensity, 1, 5);
	ImGui::SliderFloat("SSAORadius", &m_SSAORadius, 1, 7);
	ImGui::EndChild();


	ImGui::Text("Light Option");
	ImGui::BeginChild("Light Option", ImVec2(370, 130), true);

	ImGui::InputFloat3("DirAngle", m_LightDirAngle, 3);
	ImGui::SliderFloat("GlowSpeed", &m_GlowSpeed, 0.0f, 0.3f);

	ImGui::ColorEdit4("DirLight Color", m_Lightcolor);
	ImGui::ColorEdit4("DiffuseLight Color", m_DiffuseLightcolor);
	ImGui::ColorEdit4("SpecLight Color", m_SpecLightcolor);



	ImGui::EndChild();

	LightMGR::GetInstance()->UpdateDirLightColor(m_Lightcolor[0], m_Lightcolor[1], m_Lightcolor[2]);
	LightMGR::GetInstance()->UpdateDirDiffuseLightColor(m_DiffuseLightcolor[0], m_DiffuseLightcolor[1], m_DiffuseLightcolor[2]);
	LightMGR::GetInstance()->UpdateDirSpecLightColor(m_SpecLightcolor[0], m_SpecLightcolor[1], m_SpecLightcolor[2]);

	LightMGR::GetInstance()->UpdateLightAngle(m_LightDirAngle[0], m_LightDirAngle[1], m_LightDirAngle[2]);

	ImGui::End();

	
}

void cEngineScene::ForwardingUIControll()
{
	bool isOpen = true;
	ImGui::Begin("", &isOpen, ImVec2(800, 320), 0.9f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove );
	ImGui::SetWindowPos(ImVec2(620, 10));

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::Checkbox("Bumped Mapping", &m_isSSAOBlur);

	ImGui::Text("Rim Light");
	ImGui::BeginChild("Rim Light", ImVec2(370, 60), true, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SliderFloat("Rim Range", &m_GolemChaIns.m_fRimLightWidth, 0.0f, 5.0f);
	float rimColor[4] = { m_GolemChaIns.m_fRimLightColor.x , m_GolemChaIns.m_fRimLightColor.y, m_GolemChaIns.m_fRimLightColor.z,1 };
	ImGui::ColorEdit4("Rim Color", rimColor);
	m_GolemChaIns.m_fRimLightColor.x = rimColor[0];
	m_GolemChaIns.m_fRimLightColor.y = rimColor[1];
	m_GolemChaIns.m_fRimLightColor.z = rimColor[2];
	ImGui::EndChild();

	ImGui::Text("Dissolve");
	ImGui::BeginChild("Dissolve", ImVec2(370, 80), true);
	ImGui::SliderFloat("Progress", &m_GolemChaIns.m_fDissolveProgress, 0.0f, 1.0f);
	ImGui::SliderFloat("Edge", &m_GolemChaIns.m_fEdge, 0.0f, 1.0f);
	ImGui::SliderFloat("EdgeRange", &m_GolemChaIns.m_fEdgeRange, 0.0f, 1.0f);
	ImGui::EndChild();

	ImGui::Text("Directional Light");
	ImGui::BeginChild("Directional Light", ImVec2(370, 80), true);
	ImGui::ColorEdit4("DirLight Color", m_Lightcolor);
	ImGui::ColorEdit4("DiffuseLight Color", m_DiffuseLightcolor);
	ImGui::ColorEdit4("SpecLight Color", m_SpecLightcolor);
	ImGui::EndChild();

	
	ImGui::Text("Animation");
	ImGui::BeginChild("Animation", ImVec2(370, 80), true);
	ImGui::Checkbox("Stop", &m_GolemChaIns.isStop);
	ImGui::SliderFloat("Progress", &m_GolemChaIns.TimePos, 0.0f, m_GolemChaIns.GetDuration() / m_GolemChaIns.GetTicksPerSecond());
	ImGui::SliderFloat("AniSpeed", &m_GolemChaIns.AnimationSpeed, 0.1f, 2.0f);
	ImGui::EndChild();

	LightMGR::GetInstance()->UpdateDirLightColor(m_Lightcolor[0], m_Lightcolor[1], m_Lightcolor[2]);
	LightMGR::GetInstance()->UpdateDirDiffuseLightColor(m_DiffuseLightcolor[0], m_DiffuseLightcolor[1], m_DiffuseLightcolor[2]);
	LightMGR::GetInstance()->UpdateDirSpecLightColor(m_SpecLightcolor[0], m_SpecLightcolor[1], m_SpecLightcolor[2]);


	ImGui::End();
}



