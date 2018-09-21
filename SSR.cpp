#include "DXUT.h"
#include "SSR.h"
#include "Main.h"
#include "EffectMGR.h"
#include "Camera.h"

cSSR::cSSR()
{
}


cSSR::~cSSR()
{
}

void cSSR::Init(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, UINT width, UINT height)
{
	m_ThreadX = (UINT)ceilf(D3DMain::GetInstance()->GetWidth() / 16.0f);
	m_ThreadY = (UINT)ceilf(D3DMain::GetInstance()->GetHeight() / 16.0f);

	BuildTexture(pDevice, pDC);
	BuildScreenQuad(pDevice);
}

void cSSR::BuildTexture(ID3D11Device* pDevice, ID3D11DeviceContext * pDC)
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
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS  | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;


	ID3D11Texture2D* resultTex = 0;
	HR(pDevice->CreateTexture2D(&texDesc, 0, &resultTex));
	HR(pDevice->CreateShaderResourceView(resultTex, 0, &m_ResultSRV));
	HR(pDevice->CreateUnorderedAccessView(resultTex, 0, &m_ResultUAV));
	HR(pDevice->CreateRenderTargetView(resultTex, 0, &m_ResultRTV));

	SAFE_RELEASE(resultTex);
}

void cSSR::BuildScreenQuad(ID3D11Device * pDevice)
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

void cSSR::DrawSSR(ID3D11DeviceContext * pDC, ID3D11ShaderResourceView* HDRSRV, cGBuffer* Gbuffer, cCamera * cam, CXMMATRIX vp, float SSRFactor)
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
	XMMATRIX InvVP = XMMatrixInverse(nullptr, (V*P));
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// 투영 텍스처 변환 행렬 전개.
	XMMATRIX S = V * P;

	XMMATRIX InvT = XMMatrixInverse(nullptr, S);

	XMFLOAT4X4 pmat;
	XMStoreFloat4x4(&pmat, P);

	XMFLOAT4 ProjProperty = { 1.0f / pmat._11  ,1.0f / pmat._22, pmat._43,  -pmat._33 };
//*/
	XMFLOAT3 CamPos = cam->GetPos();

	EffectMGR::ScreenSpaceEffect->SetDepthMap(Gbuffer->GetDepthSRV());
	EffectMGR::ScreenSpaceEffect->SetNormalMap(Gbuffer->GetNormalSRV());
	EffectMGR::ScreenSpaceEffect->SetDiffuseMap(HDRSRV);
	EffectMGR::ScreenSpaceEffect->SetSpecMap(Gbuffer->GetSpecPowerSRV());

	EffectMGR::ScreenSpaceEffect->SetOutUAV(m_ResultUAV);
	EffectMGR::ScreenSpaceEffect->SetEyePos(CamPos);
//	EffectMGR::ScreenSpaceEffect->SetEyePos(CamPos);
	EffectMGR::ScreenSpaceEffect->SetTexToWorld(InvVP);
	EffectMGR::ScreenSpaceEffect->SetViewProj(V * P);
	EffectMGR::ScreenSpaceEffect->SetView(V);
	EffectMGR::ScreenSpaceEffect->SetProj(P);
	EffectMGR::ScreenSpaceEffect->SetInvProj(InvP);
	EffectMGR::ScreenSpaceEffect->SetProperty(ProjProperty);
	EffectMGR::ScreenSpaceEffect->SetInvView(InvV);
	EffectMGR::ScreenSpaceEffect->SetInvViewProj(InvVP);
	EffectMGR::ScreenSpaceEffect->setSSRPower(SSRFactor);
	//EffectMGR::ScreenSpaceEffect->SetWorld(InvP);

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	pDC->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
	pDC->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);

	ID3DX11EffectTechnique* tech = EffectMGR::ScreenSpaceEffect->GetTech("SSRTest");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);


	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		//UINT TreadCountX = (UINT)ceilf(m_fRTVWidth / 256.0f);

		pDC->Dispatch(m_ThreadX, m_ThreadY, 1);
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

void cSSR::DrawSSR2(ID3D11DeviceContext * pDC, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView * HDRSRV, cGBuffer * Gbuffer, cCamera * cam, CXMMATRIX vp, float SSRFactor)
{
	

	float color[] = { 0.0f,0.0f,-1.0f,1e5f };
	pDC->ClearRenderTargetView(m_ResultRTV, color);
	pDC->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* rtv[1] = { m_ResultRTV };
	pDC->OMSetRenderTargets(1, rtv, dsv);
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
	XMMATRIX InvVP = XMMatrixInverse(nullptr, (V*P));
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	// 투영 텍스처 변환 행렬 전개.
	XMMATRIX S = V * P;

	XMMATRIX InvT = XMMatrixInverse(nullptr, S);

	XMFLOAT4X4 pmat;
	XMStoreFloat4x4(&pmat, P);

	XMFLOAT4 ProjProperty = { 1.0f / pmat._11  ,1.0f / pmat._22, pmat._43,  -pmat._33 };
	//*/
	XMFLOAT3 CamPos = cam->GetPos();

	EffectMGR::ScreenSpaceEffect2->SetDepthMap(Gbuffer->GetDepthSRV());
	EffectMGR::ScreenSpaceEffect2->SetNormalMap(Gbuffer->GetNormalSRV());
	EffectMGR::ScreenSpaceEffect2->SetDiffuseMap(HDRSRV);
	EffectMGR::ScreenSpaceEffect2->SetSpecMap(Gbuffer->GetSpecPowerSRV());

	EffectMGR::ScreenSpaceEffect2->SetOutUAV(m_ResultUAV);
	EffectMGR::ScreenSpaceEffect2->SetEyePos(CamPos);
	EffectMGR::ScreenSpaceEffect2->SetWorld(V);
	EffectMGR::ScreenSpaceEffect2->SetWorldView(V);
	EffectMGR::ScreenSpaceEffect2->SetPerspectiveValue(ProjProperty);
	//	EffectMGR::ScreenSpaceEffect->SetEyePos(CamPos);
	EffectMGR::ScreenSpaceEffect2->SetTexToWorld(InvVP);
	EffectMGR::ScreenSpaceEffect2->SetViewProj(V * P);
	EffectMGR::ScreenSpaceEffect2->SetView(V);
	EffectMGR::ScreenSpaceEffect2->SetProj(P);
	EffectMGR::ScreenSpaceEffect2->SetInvProj(InvP);
	EffectMGR::ScreenSpaceEffect2->SetInvView(InvV);
	EffectMGR::ScreenSpaceEffect2->SetInvViewProj(InvVP);

	EffectMGR::ScreenSpaceEffect2->SetViewAngleThreshold(0.1f);
	EffectMGR::ScreenSpaceEffect2->SetEdgeDistThreshold(0.1f);
	EffectMGR::ScreenSpaceEffect2->SetReflectionScale(1.0f);
	EffectMGR::ScreenSpaceEffect2->SetDepthBias(0.002f);
	//EffectMGR::ScreenSpaceEffect->SetWorld(InvP);

	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	pDC->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);
	pDC->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);

	ID3DX11EffectTechnique* tech = EffectMGR::ScreenSpaceEffect2->GetTech("SSRTest3");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);


	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		//UINT TreadCountX = (UINT)ceilf(m_fRTVWidth / 256.0f);

		pDC->DrawIndexed(6, 0, 0);
	//	pDC->Dispatch(m_ThreadX, m_ThreadY, 1);
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
