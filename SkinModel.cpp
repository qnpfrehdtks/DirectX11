#include "DXUT.h"
#include "SkinModel.h"
#include "AssimpFBXLoader.h"
#include "TextureResourceMGR.h"
#include "d3dx11effect.h"
#include "EffectMGR.h"
#include "MathHelper.h"
#include "Main.h"
#include "Camera.h"
#include <DirectXTex.h>
#include "RenderState.h"
#include "ShadowMap.h"
#include "OutLineMap.h"
#include "AnimationMGR.h"
#include "PointLightShadow.h"


cSkinModel::cSkinModel() :m_MeshCount(0), m_pDevice(nullptr), m_pDC(nullptr), m_AniMGR(nullptr)
{
}

cSkinModel::~cSkinModel()
{

}

void cSkinModel::LoadFBXFile(
	ID3D11Device* device, 
	ID3D11DeviceContext* dc, 
	std::string filename, 
	std::string name, BOOL hasAni)
{
	m_pDevice = device;
	m_pDC = dc;

	std::vector<sTEXTURE> texs;

	if (hasAni) {
		m_AniMGR = new AnimationMGR();
	}

	AssimpFBXLoader::GetInstance()->LoadFBXFile(
		filename, 
		name,
		m_pDevice, 
		m_pDC,
		m_Meshes, 
		&m_SkinAni,
		texs, 
		m_AniMGR);

	m_MeshCount = m_Meshes.size();

	if( m_MeshCount > 0)
	 LoadTexture();

	sMaterial Mat;
	Mat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Mat.Diffuse = XMFLOAT4(1, 1, 1, 1.0f);
	Mat.Specular = XMFLOAT4(1, 1, 1, 12.0f);
	Mat.Reflect = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_MatArr.push_back(Mat);

	m_NormalMapGolem = TextureResourceMGR::GetInstance()->CreateSRVFromTGA(device, L"Textures/CH_NPC_MOB_RGolem_B01_N_ATR.tga");
	m_HeightMap = TextureResourceMGR::GetInstance()->CreateSRVFromTGA(device, L"Models/spnza_bricks_a_height.tga");
}

//void cSkinModel::SkiAniUpdate(float dt)
//{
//}

void cSkinModel::LoadTexture()
{
	//m_DiffuseSRVArr.reserve(m_MeshCount);
	//m_SpecularSRVArr.reserve(m_MeshCount);
	//m_NormalSRVArr.reserve(m_MeshCount);

	for (int i = 0; i < m_MeshCount; i++)
	{
		for (int j = 0; j < m_Meshes[i].GeTexCount(); j++)
		{
			sTEXTURE* tex = m_Meshes[i].GetTex(j);

			std::string temp1 = tex->fileName;
			std::wstring fileName = std::wstring(temp1.begin(), temp1.end());

			ID3D11ShaderResourceView* srv = TextureResourceMGR::GetInstance()->CreateSRVFromTGA(m_pDevice, fileName);

			if (srv != nullptr)
			{
				tex->m_RSV = srv;
			}
		}
	}
}

void cSkinModel::Draw(ID3DX11EffectTechnique* tech, int PassIdx,UINT stride)
{
	for (UINT subsetID = 0; subsetID < m_MeshCount; subsetID++)
	{
		EffectMGR::BasicE->SetMaterial(m_MatArr[subsetID]);

	//	if(m_Meshes[subsetID].GeTexCount() > 0)
		EffectMGR::BasicE->SetDiffuseMap(m_Meshes[subsetID].GetTex(0)->m_RSV);

		EffectMGR::BasicE->SetSpecMap(m_NormalMapGolem);
		EffectMGR::BasicE->SetNormalMap(m_NormalMapGolem);
		tech->GetPassByIndex(PassIdx)->Apply(0, m_pDC);

		m_Meshes[subsetID].Draw(m_pDC, stride);
	}
}

void cSkinModel::GbufferDraw(ID3DX11EffectTechnique * tech, int PassIdx, UINT stride)
{
	EffectMGR::gBufferE->SetHeightMap(m_HeightMap);

	for (UINT subsetID = 0; subsetID < m_MeshCount; subsetID++)
	{
		if (subsetID == 4) continue;

		SettingTexture(subsetID);

		tech->GetPassByIndex(PassIdx)->Apply(0, m_pDC);

		m_Meshes[subsetID].Draw(m_pDC, stride);
	}
}

void cSkinModel::GbufferDraw2(ID3DX11EffectTechnique * tech, int PassIdx, UINT stride)
{
	//EffectMGR::gBufferE->SetHeightMap(m_HeightMap);

	for (UINT subsetID = 0; subsetID < m_MeshCount; subsetID++)
	{
		SettingTexture(subsetID);

		tech->GetPassByIndex(PassIdx)->Apply(0, m_pDC);

		m_Meshes[subsetID].Draw(m_pDC, stride);
	}
}



void cSkinModel::SettingTexture(UINT idx)
{
	 UINT texIDx = m_Meshes[idx].GeTexCount();

	 EffectMGR::gBufferE->SetNormalMap(nullptr);
	 EffectMGR::gBufferE->SetDiffuseMap(nullptr);
	 EffectMGR::gBufferE->SetSpecMap(nullptr);

	 EffectMGR::gBufferE->SetIsBumped(false);
	 EffectMGR::gBufferE->SetIsSpeced(false);
	 EffectMGR::gBufferE->SetIsMask(false);
	 EffectMGR::gBufferE->SetIsPOM(false);

	// EffectMGR::gBufferE->SetDiffuseMap(m_Meshes[idx].GetTex(i)->m_RSV);
	 for (UINT i = 0; i < texIDx; i++)
	 {
		 if (m_Meshes[idx].GetTex(i)->type == aiTextureType_DIFFUSE)
		 {
			
			 EffectMGR::gBufferE->SetDiffuseMap(m_Meshes[idx].GetTex(i)->m_RSV);
		 }
		 else if (m_Meshes[idx].GetTex(i)->type == aiTextureType_SPECULAR)
		 {
			 EffectMGR::gBufferE->SetSpecMap(m_Meshes[idx].GetTex(i)->m_RSV);
			 EffectMGR::gBufferE->SetIsSpeced(true);
		 }
		 else if (m_Meshes[idx].GetTex(i)->type == aiTextureType_NORMALS)
		 {
			 EffectMGR::gBufferE->SetNormalMap(m_Meshes[idx].GetTex(i)->m_RSV);
			 EffectMGR::gBufferE->SetIsBumped(true);
		 }
		 else 
		 {
			 EffectMGR::gBufferE->SetIsMask(true);
			 EffectMGR::gBufferE->SetMaskMap(m_Meshes[idx].GetTex(i)->m_RSV);
		 }
			
	 }
}

void cSkinModel::BoneTransform(float* Time, std::vector<XMFLOAT4X4>& FinalTransform)
{
	m_AniMGR->BoneTransform(Time, FinalTransform);
}



void sSkinModellInstance::Draw(ID3D11DeviceContext* pDC, cShadowMap* shadowMap, cOutLineMap* pOutLineMap)
{
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::SkinInputLayout);

	const sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];

	ID3DX11EffectTechnique* tech = EffectMGR::BasicE->GetTech(TechName);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	XMMATRIX scale = XMMatrixScaling(ToonTickness, ToonTickness, ToonTickness);



	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pDC->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		 //=================== Dissolve Shader ============================
		EffectMGR::BasicE->SetProgress(m_fDissolveProgress);
		EffectMGR::BasicE->SetEdge(m_fEdge);
		EffectMGR::BasicE->SetEdgeRange(m_fEdgeRange);
		
		if (m_pDissolveSRV)
		{
			EffectMGR::BasicE->SetDissolveMap(m_pDissolveSRV);
			EffectMGR::BasicE->SetDissolveColorMap(m_pDissolveRampSRV);
		}

		//=================== Normal + Depth Debuging Shader ============================

		if (pOutLineMap != nullptr)
		{
			XMMATRIX outLineTransform = XMLoadFloat4x4(&pOutLineMap->GetOutLineTransform());
			EffectMGR::BasicE->SetOutLine(pOutLineMap->GetOutLineSRVMap());
			EffectMGR::BasicE->SetOutLineTransform(outLineTransform);
		}

		// ==================== Rim Light ============================
		EffectMGR::BasicE->SetRimColor(m_fRimLightColor);
		EffectMGR::BasicE->SetRimWidth(m_fRimLightWidth);

		// ====================== Draw Setting =====================

		XMMATRIX world = XMLoadFloat4x4(&World);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

		XMMATRIX wvp = world * D3DMain::GetInstance()->GetCam()->GetViewMat() * D3DMain::GetInstance()->GetCam()->GetProjMat();
		
		EffectMGR::BasicE->SetEyePosW(D3DMain::GetInstance()->GetCam()->GetPos());
		EffectMGR::BasicE->SetWorld(world);
		EffectMGR::BasicE->SetToonWVP(scale);
		EffectMGR::BasicE->SetWorldViewProj(wvp);
		EffectMGR::BasicE->SetBoneTransform(&FinalTransforms[0], FinalTransforms.size());
    
		EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);
		//EffectMGR::BasicE->SetSpecExp(250.0f);
		//EffectMGR::BasicE->SetSpecInt(0.25f);
		EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);
		EffectMGR::BasicE->SetDirLights(&dirLight);
		EffectMGR::BasicE->SetTexTransform(XMMatrixIdentity());
		EffectMGR::BasicE->SetShadowTransform(world * XMLoadFloat4x4(&shadowMap->GetShadowTransform()));
		EffectMGR::BasicE->SetShadowMap(shadowMap->GetShadowMap());

		Model->Draw(tech, p, sizeof(Vertexs::sSkinnedVertex));
	}

	pDC->RSSetState(0);

}

void sSkinModellInstance::SkinnedGBufferDraw(ID3D11DeviceContext * pDC, cShadowMap * shadowMap, cPointLightShadowMap* ptShadowMap, cOutLineMap * pOutLineMap,bool isPCF)
{
	pDC->RSSetState(0);

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::SkinInputLayout);

	const sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];

	ID3DX11EffectTechnique* tech = EffectMGR::gBufferE->GetTech("GbufferSkinNormal");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	cCamera* cam = D3DMain::GetInstance()->GetCam();


	XMMATRIX W = XMLoadFloat4x4(&World);
	XMMATRIX V = cam->GetViewMat();
	XMMATRIX P = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
	XMMATRIX worldInvTransposeView = worldInvTranspose * V;
	XMMATRIX WV = W * V;
	XMMATRIX WVP = WV * P;
	XMMATRIX VP = V * P;
	// ===================== World, Proj, View... ===================

	EffectMGR::gBufferE->SetShadowMap(shadowMap->GetShadowMap());

	EffectMGR::gBufferE->SetWorld(W);
	EffectMGR::gBufferE->SetView(V);
	EffectMGR::gBufferE->SetEyePosW(cam->GetPos());
	EffectMGR::gBufferE->SetWorldViewProj(WVP);
	EffectMGR::gBufferE->SetWorldView(WV);
	EffectMGR::gBufferE->SetViewProj(VP);

	EffectMGR::gBufferE->SetWorldInvTranspose(worldInvTranspose);
	EffectMGR::gBufferE->SetShadowTransform(W * XMLoadFloat4x4(&shadowMap->GetShadowTransform()));
	EffectMGR::gBufferE->SetSpecExp(8.0f);
	EffectMGR::gBufferE->SetSpecInt(0.25f);
	EffectMGR::gBufferE->SetBoneTransform(&FinalTransforms[0], FinalTransforms.size());

	EffectMGR::gBufferE->SetIsBumped(true);
	EffectMGR::gBufferE->SetIsSpeced(true);
	EffectMGR::gBufferE->SetIsMask(false);
	EffectMGR::gBufferE->SetIsShadowed(isPCF);
	
	//EffectMGR::gBufferE->SetCubeMap(ptShadowMap->GetCubeMap());

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//	pDC->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		//=================== Dissolve Shader ============================
		//	EffectMGR::BasicE->SetProgress(m_fDissolveProgress);
		//	EffectMGR::BasicE->SetEdge(m_fEdge);
		//	EffectMGR::BasicE->SetEdgeRange(m_fEdgeRange);

		//	EffectMGR::BasicE->SetDissolveMap(m_pDissolveSRV);
		//	EffectMGR::BasicE->SetDissolveColorMap(m_pDissolveRampSRV);

		//=================== Normal + Depth Debuging Shader ============================

		//if (pOutLineMap != nullptr)
		//{
		//	XMMATRIX outLineTransform = XMLoadFloat4x4(&pOutLineMap->GetOutLineTransform());
		//	EffectMGR::BasicE->SetOutLine(pOutLineMap->GetOutLineSRVMap());
		//	EffectMGR::BasicE->SetOutLineTransform(outLineTransform);
		//}

		// ==================== Rim Light ============================
		//	EffectMGR::BasicE->SetRimColor(m_fRimLightColor);
		//	EffectMGR::BasicE->SetRimWidth(m_fRimLightWidth);

		// ====================== Draw Setting =====================

		//EffectMGR::gBufferE->SetWorldInvTranspose(worldInvTranspose);
		//	EffectMGR::gBufferE->SetDirLights(&dirLight->dirLight);
		//	EffectMGR::gBufferE->SetTexTransform(XMMatrixIdentity());
		//	EffectMGR::gBufferE->SetShadowTransform(world * XMLoadFloat4x4(&shadowMap->GetShadowTransform()));

		Model->GbufferDraw2(tech, p, sizeof(Vertexs::sSkinnedVertex));
	}

	pDC->RSSetState(0);
}

void sSkinModellInstance::GBufferDraw(ID3D11DeviceContext * pDC,  cShadowMap * shadowMap, cOutLineMap * pOutLineMap, CXMMATRIX vp, float heightScale)
{
	pDC->RSSetState(0);

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::SkinInputLayout);

	const sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];


	cCamera* cam = D3DMain::GetInstance()->GetCam();

	XMMATRIX W = XMLoadFloat4x4(&World);
	XMMATRIX V = cam->GetViewMat();
	XMMATRIX P = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
	//XMMATRIX worldInvTransposeView = worldInvTranspose * V;
	XMMATRIX WV = W * V;
	XMMATRIX VP = V * P;
	XMMATRIX WVP = W * V * P;
//	cam->SetVP(VP);
	// ===================== World, Proj, View... ===================
//	EffectMGR::gBufferE->SetEyePosW(D3DMain::GetInstance()->GetCam()->GetPos());

	EffectMGR::gBufferE->SetShadowMap(shadowMap->GetShadowMap());

	EffectMGR::gBufferE->SetWorld(W);
	EffectMGR::gBufferE->SetView(V);
	EffectMGR::gBufferE->SetEyePosW(cam->GetPos());
	EffectMGR::gBufferE->SetWorldViewProj(WVP);
	EffectMGR::gBufferE->SetWorldView(WV);
	EffectMGR::gBufferE->SetViewProj(VP);

	EffectMGR::gBufferE->SetWorldInvTranspose(worldInvTranspose);
	EffectMGR::gBufferE->SetShadowTransform(W * XMLoadFloat4x4(&shadowMap->GetShadowTransform()));
	EffectMGR::gBufferE->SetSpecExp(1.0f);
	EffectMGR::gBufferE->SetSpecInt(0.25f);
	EffectMGR::gBufferE->SetHeightScale(heightScale);

	EffectMGR::gBufferE->SetIsBumped(true);
	EffectMGR::gBufferE->SetIsSpeced(true);
	EffectMGR::gBufferE->SetIsMask(true);
	EffectMGR::gBufferE->SetIsShadowed(true);

	ID3DX11EffectTechnique* tech = EffectMGR::gBufferE->GetTech("GbufferBasicNormal");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Model->GbufferDraw(tech, p, sizeof(Vertexs::sSkinnedVertex));
	}
//	pDC->OMSetDepthStencilState(0, 0);
	pDC->RSSetState(0);
}



void sSkinModellInstance::Update(float dt)
{
	
	
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		 TechName = "SkinDirLightNoRimNoBumped";
	}
	else if (GetAsyncKeyState('E') & 0x8000)
	{
		TechName = "SkinDirLightBumped";
	}

	if (isStop) {
		TimePos += 0;
	}
	else {
		TimePos += (dt * AnimationSpeed);
	}

	Model->BoneTransform(&TimePos, FinalTransforms);

//	Model->BoneTransform(TimePos, FinalTransforms);
	//Model->GetAniMGR()->Bone
	
	//AssimpFBXLoader::GetInstance()->BoneTransform(TimePos, FinalTransforms);
}
