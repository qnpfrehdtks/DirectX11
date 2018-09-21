#include "DXUT.h"
#include "SSAO.h"
#include "MathHelper.h"
#include "Camera.h"
#include "Main.h"
#include "Vertex.h"
#include "EffectMGR.h"
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

cSSAO::cSSAO()
	: m_pIB(nullptr),
	m_pRandomVectorSRV(nullptr), 
	m_pRTV01(nullptr), m_pRTV02(nullptr), m_pSSAOMap01(nullptr), m_pSSAOMap02(nullptr), m_pVB(nullptr)
{
	ZeroMemory(&m_SSAOViewPort, sizeof(D3D11_VIEWPORT));
}


cSSAO::~cSSAO()
{
}

void cSSAO::Init(float Width, float Height, ID3D11Device * pDevice, ID3D11DeviceContext * pDC, cCamera* cam) 
{
	m_fRTVWidth = Width;
	m_fRTVHeight = Height;

	m_ThreadX = (UINT)ceilf(m_fRTVWidth / 256.0f);
	m_ThreadY = (UINT)ceilf(m_fRTVHeight / 256.0f);

	A = cam->GetFarZ() / (cam->GetFarZ() - cam->GetNearZ());
	B = -cam->GetNearZ() / (cam->GetFarZ() - cam->GetNearZ());

	
	SetFrustum(cam->GetFovY(), cam->GetFarZ());
	BuildTextureCS(Width, Height, pDevice, pDC);

	SetScreenQuad(pDevice);
	//BuildOffsetVector(pDevice, pDC);

	BuildRandomVectorTexture(pDevice, pDC);


	EffectMGR::SsaoE->SetFrustumCorners(m_Frustum);
	EffectMGR::SsaoE->SetRandomMap(m_pRandomVectorSRV);
	EffectMGR::SsaoE->SetOffsetVector(m_Offsets);

	
}




void cSSAO::BuildTexture(float Width, float Height, ID3D11Device* pDevice, ID3D11DeviceContext * pDC)
{
	// We render to ambient map at half the resolution.
	m_SSAOViewPort.TopLeftX = 0.0f;
	m_SSAOViewPort.TopLeftY = 0.0f;
	m_SSAOViewPort.Width = Width / 2;
	m_SSAOViewPort.Height = Height / 2;
	m_SSAOViewPort.MinDepth = 0.0f;
	m_SSAOViewPort.MaxDepth = 1.0f;

	m_TexHeight = 1.0f / m_SSAOViewPort.Height;
	m_TexWidth = 1.0f / m_SSAOViewPort.Width;


	/*typedef struct D3D11_TEXTURE2D_DESC
	{
		UINT Width;
		UINT Height;
		UINT MipLevels;
		UINT ArraySize;
		DXGI_FORMAT Format;
		DXGI_SAMPLE_DESC SampleDesc;
		D3D11_USAGE Usage;
		UINT BindFlags;
		UINT CPUAccessFlags;
		UINT MiscFlags;
	} 	D3D11_TEXTURE2D_DESC;*/

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// SSAO 에 쓰는 Texture는 기존 크기의 1/4 크기로 줄여서 생성하자.
	// 크기를 줄이고, Format 형식을 FLOAT 형식으로
	texDesc.Width = Width / 2;
	texDesc.Height = Height / 2;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;

	ID3D11Texture2D* ambientTex0 = 0;
	HR(pDevice->CreateTexture2D(&texDesc, 0, &ambientTex0));
	HR(pDevice->CreateShaderResourceView(ambientTex0, 0, &m_pSSAOMap01));
	HR(pDevice->CreateRenderTargetView(ambientTex0, 0, &m_pRTV01));

	ID3D11Texture2D* ambientTex1 = 0;
	HR(pDevice->CreateTexture2D(&texDesc, 0, &ambientTex1));
	HR(pDevice->CreateShaderResourceView(ambientTex1, 0, &m_pSSAOMap02));
	HR(pDevice->CreateRenderTargetView(ambientTex1, 0, &m_pRTV02));

	SAFE_RELEASE(ambientTex0);
	SAFE_RELEASE(ambientTex1);

}

void cSSAO::BuildTextureCS(float Width, float Height, ID3D11Device * pDevice, ID3D11DeviceContext * pDC)
{
	// We render to ambient map at half the resolution.
	m_SSAOViewPort.TopLeftX = 0.0f;
	m_SSAOViewPort.TopLeftY = 0.0f;
	m_SSAOViewPort.Width = Width /2;
	m_SSAOViewPort.Height = Height / 2;
	m_SSAOViewPort.MinDepth = 0.0f;
	m_SSAOViewPort.MaxDepth = 1.0f;

	m_TexHeight = 1.0f / m_SSAOViewPort.Height;
	m_TexWidth = 1.0f / m_SSAOViewPort.Width;


	/*typedef struct D3D11_TEXTURE2D_DESC
	{
	UINT Width;
	UINT Height;
	UINT MipLevels;
	UINT ArraySize;
	DXGI_FORMAT Format;
	DXGI_SAMPLE_DESC SampleDesc;
	D3D11_USAGE Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
	} 	D3D11_TEXTURE2D_DESC;*/

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;


	// SSAO 에 쓰는 Texture는 기존 크기의 1/4 크기로 줄여서 생성하자.
	// 크기를 줄이고, Format 형식을 FLOAT 형식으로
	texDesc.Width = Width / 2;
	texDesc.Height = Height / 2;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	// 이번에는 계산 쉐이더의 출력을 위해, UAV로 묶어준다.
	/*D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	uavd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavd.Texture2D.MipSlice = 0;*/
	// View 생성 후에는 Effect에 묶어 주어야 하는데...


	ID3D11Texture2D* ambientTex0 = 0;
	HR(pDevice->CreateTexture2D(&texDesc, 0, &ambientTex0));
	HR(pDevice->CreateShaderResourceView(ambientTex0, 0, &m_pSSAOMap01));
	HR(pDevice->CreateRenderTargetView(ambientTex0, 0, &m_pRTV01));
	HR(pDevice->CreateUnorderedAccessView(ambientTex0, 0, &m_pSSAOUAVMap01));

	ID3D11Texture2D* ambientTex1 = 0;
	HR(pDevice->CreateTexture2D(&texDesc, 0, &ambientTex1));
	HR(pDevice->CreateShaderResourceView(ambientTex1, 0, &m_pSSAOMap02));
	HR(pDevice->CreateRenderTargetView(ambientTex1, 0, &m_pRTV02));
	HR(pDevice->CreateUnorderedAccessView(ambientTex1, 0, &m_pSSAOUAVMap02));

	
}

void cSSAO::BlurSSAOMapCS(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, ID3D11ShaderResourceView * depthSRV, ID3D11ShaderResourceView * normalSRV)
{
	ID3D11RenderTargetView* rtv[1] = { nullptr };
	pDC->OMSetRenderTargets(1, rtv, 0);

	XMFLOAT4 ProjAB;
	ProjAB.x = A;
	ProjAB.y = B;

	ID3DX11EffectTechnique* horz = EffectMGR::SSAOCSBlurE->GetTech("HorzBlur");
	ID3DX11EffectTechnique* vert = EffectMGR::SSAOCSBlurE->GetTech("VertBlur");

	D3DX11_TECHNIQUE_DESC tech;
	EffectMGR::SSAOCSBlurE->SetDepthSRV(depthSRV);
	EffectMGR::SSAOCSBlurE->SetNormalSRV(normalSRV);
	EffectMGR::SSAOCSBlurE->SetVectorAB(ProjAB);

	for (int i = 0; i < 2; i++)
	{
		horz->GetDesc(&tech);

		for (int j = 0; j < tech.Passes; j++){
			
			EffectMGR::SSAOCSBlurE->SetInputSRV(m_pSSAOMap01);
			EffectMGR::SSAOCSBlurE->SetOutUAV(m_pSSAOUAVMap02);

			horz->GetPassByIndex(j)->Apply(0, pDC);
			pDC->Dispatch(m_ThreadX, m_fRTVHeight, 1);

		}

		// 계산 쉐이더에서 입력 텍스처를 계산 쉐이더에서 우선 띠어내고.
		ID3D11ShaderResourceView* srv[1] = { 0 };
		pDC->CSSetShaderResources(0, 1, srv);


		// 계산 쉐이더에서 출력 을 다음 패스의 입력으로 사용한다.
		// 하나의 자원을 동시에 읽고 쓸수 없다.
		ID3D11UnorderedAccessView* uav[1] = { 0 };
		pDC->CSSetUnorderedAccessViews(0, 1, uav, 0);

		vert->GetDesc(&tech);

		for (int j = 0; j < tech.Passes; j++) {

			EffectMGR::SSAOCSBlurE->SetInputSRV(m_pSSAOMap02);
			EffectMGR::SSAOCSBlurE->SetOutUAV(m_pSSAOUAVMap01);

			vert->GetPassByIndex(j)->Apply(0, pDC);

			pDC->Dispatch(m_fRTVWidth, m_ThreadY, 1);

		}
	}



}



void cSSAO::BuildRandomVectorTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pDC)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * sizeof(DWORD);

	static DWORD color[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			DWORD newColor[4];
			newColor[0] = 0.0f;
			newColor[1] = rand() / 255.0f;
			newColor[2] = rand() / 255.0f;
			newColor[3] = rand() / 255.0f;
			color[i * 256 + j] = D3DCOLOR_ARGB(newColor[0], newColor[1], newColor[2], newColor[3]);

			
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	HR(pDevice->CreateTexture2D(&texDesc, &initData, &tex));

	HR(pDevice->CreateShaderResourceView(tex, 0, &m_pRandomVectorSRV));

	// view saves a reference.
	SAFE_RELEASE(tex);

}

void cSSAO::CreateSSAOMap(ID3D11DeviceContext* pDC, cCamera * camera, ID3D11ShaderResourceView* depthSRV, ID3D11ShaderResourceView* normalSRV,
	float Radius, float Intensity)
{
	ID3D11RenderTargetView* renderTargets[1] = { m_pRTV01 };
	//// Amient Map을 렌더 타겟으로 묶고, 깊이 스텐실을 묶지 않는다.
	pDC->OMSetRenderTargets(1, renderTargets, 0);

	//// 메 프레임 마다 RTV를 초기화 시켜준다.
	pDC->ClearRenderTargetView(m_pRTV01, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	//// ViewPort 설정.
	pDC->RSSetViewports(1, &m_SSAOViewPort);


	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	//cCamera* cam = D3DMain::GetInstance()->GetCam();

	ID3DX11EffectTechnique* tech = EffectMGR::SsaoE->GetTech("Ssao");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);

	//XMFLOAT4 ProjPro = { 0,0,A,B };

	// ===========================================
	// View -> Proj -> Tex 로 가기 위한, 행렬식.


//	XMMATRIX world = XMMatrixIdentity();
//	XMMATRIX WorldInv = MathHelper::InverseTranspose(world);

	XMMATRIX V = camera->GetViewMat();
	XMMATRIX P = camera->GetProjMat();



	// =============================================
	//EffectMGR::SsaoE->SetFrustumCorners(m_Frustum);
	//EffectMGR::SsaoE->SetRandomMap(m_pRandomVectorSRV);
 //   EffectMGR::SsaoE->SetOffsetVector(m_Offsets);
	EffectMGR::SsaoE->SetDepthMap(depthSRV);
	EffectMGR::SsaoE->SetNormalMap(normalSRV);
	EffectMGR::SsaoE->SetIntesity(Intensity);
	EffectMGR::SsaoE->SetRadius(Radius);
//	EffectMGR::SsaoE->SetProjProperty(ProjPro);
	EffectMGR::SsaoE->SetView(V);
	EffectMGR::SsaoE->SetInvProj(XMMatrixInverse(0, P));

	pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pDC->IASetVertexBuffers(0,1,&m_pVB,&stride, &offset);
	pDC->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R16_UINT, offset);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		pDC->DrawIndexed(6, 0, 0);
	}
}

void cSSAO::BlurSSAOMap(ID3D11Device * pDevice, 
	ID3D11DeviceContext * pDC, 
	cCamera * camera, 
	bool isVert, 
	ID3D11ShaderResourceView * inputSRV, 
	ID3D11RenderTargetView* outputRTV, ID3D11ShaderResourceView * depthSRV, ID3D11ShaderResourceView * normalSRV)
{

	XMFLOAT4 ProjAB;
	ProjAB.x = A;
	ProjAB.y = B;

	ID3D11RenderTargetView* renderTargets[1] = { outputRTV };
	pDC->OMSetRenderTargets(1, renderTargets, 0);
	pDC->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&Colors::Black));
	pDC->RSSetViewports(1, &m_SSAOViewPort);

	UINT stride = sizeof(Vertexs::sBasicVertex);
	UINT offset = 0;

	//cCamera* cam = D3DMain::GetInstance()->GetCam();
	ID3DX11EffectTechnique* tech;

	if(isVert)
	 tech = EffectMGR::SSAOBlurE->GetTech("EdgeBlurVert");
	else
	 tech = EffectMGR::SSAOBlurE->GetTech("EdgeBlurHorz");

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	// ===========================================
	// View -> Proj -> Tex 로 가기 위한, 행렬식.

	XMMATRIX V = camera->GetViewMat();
	// =============================================
	EffectMGR::SSAOBlurE->SetDepthMap(depthSRV);
	EffectMGR::SSAOBlurE->SetNormalMap(normalSRV);
	EffectMGR::SSAOBlurE->SetInputMap(inputSRV);
	EffectMGR::SSAOBlurE->SetView(V);
	EffectMGR::SSAOBlurE->SetTexHeight(m_TexHeight);
	EffectMGR::SSAOBlurE->SetTexWidth(m_TexWidth);
	EffectMGR::SSAOBlurE->SetVectorAB(ProjAB);

	pDC->IASetInputLayout(InputLayOut::BasicInputLayout);
	pDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pDC->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
	pDC->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R16_UINT, offset);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, pDC);
		pDC->DrawIndexed(6, 0, 0);

		EffectMGR::SSAOBlurE->SetInputMap(nullptr);
		tech->GetPassByIndex(p)->Apply(0, pDC);
	}
}



void cSSAO::Blur(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, cCamera * camera, UINT blurCount, ID3D11ShaderResourceView * depthSRV, ID3D11ShaderResourceView * normalSRV)
{
	for (int i = 0; i < blurCount; i++)
	{
		BlurSSAOMap(pDevice, pDC, camera, true, m_pSSAOMap01, m_pRTV02, depthSRV, normalSRV);
		BlurSSAOMap(pDevice, pDC, camera, false, m_pSSAOMap02, m_pRTV01, depthSRV, normalSRV);
	}
}



void cSSAO::BuildOffsetVector(ID3D11Device * pDevice, ID3D11DeviceContext * pDC)
{
	// 6면체 각 점
	m_Offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_Offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_Offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_Offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_Offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_Offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_Offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_Offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 면체의 각 면의 중심.
	m_Offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_Offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_Offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_Offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_Offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_Offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Rnadom 볌위로 Sclae 잡아줌.
		float s = MathHelper::RandF(0.25f, 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&m_Offsets[i]));

		XMStoreFloat4(&m_Offsets[i], v);
	}

}

void cSSAO::SetFrustum(float fov, float farZ)
{
	cCamera* cam = D3DMain::GetInstance()->GetCam();

	A = cam->GetFarZ() / (cam->GetFarZ() - cam->GetNearZ());
	B = (-cam->GetNearZ() / (cam->GetFarZ() - cam->GetNearZ()));

	float aspect = D3DMain::GetInstance()->GetWidth() / D3DMain::GetInstance()->GetHeight();
	float halfHeight = tanf(fov * 0.5f) * farZ;
	float halfWidth = aspect * halfHeight;

	// 1----------2
	// |          |
	// |          |
	// |          |
	// |          |
	// 0----------3
	m_Frustum[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_Frustum[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_Frustum[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	m_Frustum[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}

void cSSAO::SetScreenQuad(ID3D11Device* pDevice)
{
	//  Far Plane을 만들기 위한 메소드
	sBasicVertex v[4];

	//v[0].pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	//v[1].pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	//v[2].pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
	//v[3].pos = XMFLOAT3(1.0f, -1.0f, 0.0f);
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
	HR(pDevice->CreateBuffer(&vbd, &vinitData, &m_pVB));

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

	HR(pDevice->CreateBuffer(&ibd, &iinitData, &m_pIB));
}
