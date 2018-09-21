#include "DXUT.h"
#include "TileDeferredingShading.h"
#include "EffectMGR.h"
#include "Vertex.h"
#include "Camera.h"
#include "Main.h"
#include "Gbuffer.h"
#include "MathHelper.h"
#include "RenderState.h"
#include "LightMGR.h"
#include "ShadowMap.h"


TileDeferredingShading::TileDeferredingShading() : m_ThreadX(0), m_ThreadY(0)
{
}


TileDeferredingShading::~TileDeferredingShading()
{
}

void TileDeferredingShading::Init(ID3D11Device * device, ID3D11DeviceContext * pDC)
{
	const cCamera* cam = D3DMain::GetInstance()->GetCam();

	m_ThreadX = (UINT)ceilf(D3DMain::GetInstance()->GetWidth() / 16.0f);
	m_ThreadY = (UINT)ceilf(D3DMain::GetInstance()->GetHeight() / 16.0f);

	m_PtLights = LightMGR::GetInstance()->GetPtLights();
	m_DirLights = LightMGR::GetInstance()->GetDirLights();

	BuildTextureView(device, pDC);
	BuildScreenQuad(device);
	BuildStructureBuffer(device, pDC);
}

void TileDeferredingShading::BuildTextureView(ID3D11Device * device, ID3D11DeviceContext * pDC)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = D3DMain::GetInstance()->GetWidth();
	texDesc.Height = D3DMain::GetInstance()->GetHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE  | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;


	ID3D11Texture2D* resultTex = 0;
	HR(device->CreateTexture2D(&texDesc, 0, &resultTex));
	HR(device->CreateShaderResourceView(resultTex, 0, &m_ResultSRV));
	HR(device->CreateUnorderedAccessView(resultTex, 0, &m_ResultUAV));

	SAFE_RELEASE(resultTex);
}

void TileDeferredingShading::BuildStructureBuffer(ID3D11Device * device, ID3D11DeviceContext * pDC)
{

	XMMATRIX porjMat =  D3DMain::GetInstance()->GetCam()->GetProjMat();
	XMFLOAT4X4 projMat2;
	XMStoreFloat4x4(&projMat2, porjMat);

	A = 1.0f / projMat2._11;
	B = 1.0f / projMat2._22;


	D3D11_BUFFER_DESC inputDesc;
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = sizeof(sPtLight) * 800;
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputDesc.StructureByteStride = sizeof(sPtLight);
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA vinitDataA;
	vinitDataA.pSysMem = (&(*m_PtLights)[0]);

//	ID3D11Buffer* LightBuffer = nullptr;
	HR(device->CreateBuffer(&inputDesc, &vinitDataA, &m_PtLightBuffer));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.NumElements = 800;

	device->CreateShaderResourceView(m_PtLightBuffer, &srvDesc, &m_LightSRV);

	//SAFE_RELEASE(LightBuffer);


	// ============================================
	//                 Direct Light
	// ============================================

	//const sDirLight dirLight = *LightMGR::GetInstance()->GetDirLights();

	D3D11_BUFFER_DESC inputDesc2;
	inputDesc2.Usage = D3D11_USAGE_DEFAULT;
	inputDesc2.ByteWidth = sizeof(sDirLight) * 1;
	inputDesc2.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc2.CPUAccessFlags = 0;
	inputDesc2.StructureByteStride = sizeof(sDirLight);
	inputDesc2.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA vinitDataB;
	vinitDataB.pSysMem = &((*m_DirLights)[0]);

	//ID3D11Buffer* DirBuffer = nullptr;
	HR(device->CreateBuffer(&inputDesc2, &vinitDataB, &m_DirectLightBuffer));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc2;
	srvDesc2.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc2.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc2.BufferEx.FirstElement = 0;
	srvDesc2.BufferEx.NumElements = 1;

	device->CreateShaderResourceView(m_DirectLightBuffer, &srvDesc2, &m_DirectLightSRV);


//	SAFE_RELEASE(DirBuffer);

}

void TileDeferredingShading::LightPass(
	ID3D11DeviceContext * pDC, 
	cGBuffer * Gbuffer, 
	cCamera * cam, 
	LPCSTR str, 
	cShadowMap * shadowMap, cPointLightShadowMap* ptShadowMap, ID3D11ShaderResourceView * ssaoMap, CXMMATRIX vp, int mode)
{

	ID3D11RenderTargetView* rtv[1] = { nullptr };
	pDC->OMSetRenderTargets(1, rtv, 0);
//	const sDirectionalLight* dirLight = LightMGR::GetInstance()->GetDirLights()[0].get();

	//sPointLight* ptLights;
	//std::vector<sPointLight*> pts = LightMGR::GetInstance()->GetPtLightsVec();
	//sPtLight vec[100];
	//for (int i = 0; i < pts.size(); i++)
	//{
	//	vec[i] = pts[i]->ptLight;
	//}

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	XMMATRIX P = cam->GetProjMat();
	XMMATRIX V = cam->GetViewMat();

	//XMMatrixDeterminant(V);
	 XMMATRIX InvV = XMMatrixInverse(nullptr, V);
	 XMMATRIX InvP = XMMatrixInverse(nullptr, P);
	XMFLOAT4X4 pmat;
	XMStoreFloat4x4(&pmat, P);

	XMFLOAT4 ProjProperty = { A,B, pmat._43,  -pmat._33 };

	XMFLOAT3 CamPos = cam->GetPos();

	EffectMGR::TileE->SetDepthMap(Gbuffer->GetDepthSRV());
	EffectMGR::TileE->SetDiffuseSpecIntMap(Gbuffer->GetDiffuseSRV());
	EffectMGR::TileE->SetNormalMap(Gbuffer->GetNormalSRV());
	EffectMGR::TileE->SetSpecPowMap(Gbuffer->GetSpecPowerSRV());
	EffectMGR::TileE->SetSSAOMap(ssaoMap);

	EffectMGR::TileE->SetLightSRV(m_LightSRV);
	EffectMGR::TileE->SetDirLightSRV(m_DirectLightSRV);
	EffectMGR::TileE->SetLightCount(800);

	EffectMGR::TileE->SetOutUAV(m_ResultUAV);

	EffectMGR::TileE->SetWorldViewProj(XMMatrixIdentity());
	EffectMGR::TileE->SetEyePosW(CamPos);

	EffectMGR::TileE->SetView(V);
	EffectMGR::TileE->SetProj(P);
	EffectMGR::TileE->SetViewProj(V * P);
	EffectMGR::TileE->SetInvProj(InvP);
	EffectMGR::TileE->SetInvView(InvV);
	EffectMGR::TileE->SetProjProperty(ProjProperty);


	EffectMGR::TileE->SetRenderMode(mode);


//	EffectMGR::TileE->SetInvViewProj(InvVP);
//	EffectMGR::TileE->SetSSAOMap(ssaoMap);

//	EffectMGR::TileE->SetShadowMap(shadowMap->GetShadowMap());
//	EffectMGR::TileE->SetShadowTransform(XMLoadFloat4x4(&shadowMap->GetShadowTransform()));

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	pDC->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
	pDC->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);

	ID3DX11EffectTechnique* tech = EffectMGR::TileE->GetTech(str);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		//UINT TreadCountX = (UINT)ceilf(m_fRTVWidth / 256.0f);

		pDC->Dispatch(m_ThreadX , m_ThreadY  , 1);
	}


	pDC->CSSetShader(0, 0, 0);
	// 계산 쉐이더에서 입력 텍스처를 계산 쉐이더에서 우선 띠어내고.
	ID3D11ShaderResourceView* srv[1] = { 0 };
	pDC->CSSetShaderResources(0, 1, srv);


	// 계산 쉐이더에서 출력 을 다음 패스의 입력으로 사용한다.
	// 하나의 자원을 동시에 읽고 쓸수 없다.
	ID3D11UnorderedAccessView* uav[1] = { 0 };
	pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);


}

void TileDeferredingShading::BuildScreenQuad(ID3D11Device * pDevice)
{
	//  Far Plane을 만들기 위한 메소드
	sBasicVertex v[4];

	v[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].Pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	v[2].Pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
	v[3].Pos = XMFLOAT3(1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].Tex = XMFLOAT2(0.0f, 1.0f);
	v[1].Tex = XMFLOAT2(0.0f, 0.0f);
	v[2].Tex = XMFLOAT2(1.0f, 0.0f);
	v[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sBasicVertex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	// Buffer 생성
	HR(pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	USHORT indices[6] =
	{
		0,1,2,
		0,2,3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	HR(pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));
}

void TileDeferredingShading::LightUpdate(ID3D11Device * device, ID3D11DeviceContext * pDC)
{
	pDC->UpdateSubresource(m_PtLightBuffer, 0, NULL, &(*m_PtLights)[0], 0, 0);
	pDC->UpdateSubresource(m_DirectLightBuffer, 0, NULL, &(*m_DirLights)[0], 0, 0);
}
