#include "DXUT.h"
#include "PointLightShadow.h"
#include "EffectMGR.h"
#include "MathHelper.h"

static const int CubeMapSize = 256;

cPointLightShadowMap::cPointLightShadowMap()
{
}


cPointLightShadowMap::~cPointLightShadowMap()
{
}

void cPointLightShadowMap::Init(ID3D11Device * device, ID3D11DeviceContext * dc, UINT width, UINT height)
{
	CreateTextureView(device, dc);
}

void cPointLightShadowMap::CreateTextureView(ID3D11Device * device, ID3D11DeviceContext * dc)
{
	// Allocate the depth stencil target
	D3D11_TEXTURE2D_DESC dtd = {
		CubeMapSize, //UINT Width;
		CubeMapSize, //UINT Height;
		1, //UINT MipLevels;
		1, //UINT ArraySize;
		   //DXGI_FORMAT_R24G8_TYPELESS,
		   DXGI_FORMAT_R32_TYPELESS, //DXGI_FORMAT Format;
		   1, //DXGI_SAMPLE_DESC SampleDesc;
		   0,
		   D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		   D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		   0,//UINT CPUAccessFlags;
		   0//UINT MiscFlags;    
	};
	D3D11_DEPTH_STENCIL_VIEW_DESC descDepthView =
	{
		// DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0
	};
	D3D11_SHADER_RESOURCE_VIEW_DESC descShaderView =
	{
		//DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		DXGI_FORMAT_R32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	descShaderView.Texture2D.MipLevels = 1;


	// Allocate the point shadow targets and views
	dtd.ArraySize = 6;
	dtd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	descDepthView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	descDepthView.Texture2DArray.FirstArraySlice = 0;
	descDepthView.Texture2DArray.ArraySize = 6;

	descShaderView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	descShaderView.TextureCube.MipLevels = 1;
	descShaderView.TextureCube.MostDetailedMip = 0;
	for (int i = 0; i < 1; i++)
	{
		HR(device->CreateTexture2D(&dtd, NULL, &m_Tex[i]));
		HR(device->CreateDepthStencilView(m_Tex[i], &descDepthView, &m_DSV));
		HR(device->CreateShaderResourceView(m_Tex[i], &descShaderView, &m_SRV));
	}

	
	for (int i = 0; i < 6; i++) {
		m_ViewPort[i].TopLeftX = 0.0f;
		m_ViewPort[i].TopLeftY = 0.0f;
		m_ViewPort[i].Width = (float)m_Width;
		m_ViewPort[i].Height = (float)m_Height;
		m_ViewPort[i].MinDepth = 0.0f;
		m_ViewPort[i].MaxDepth = 1.0f;

	}
}

void cPointLightShadowMap::BuildCubeFaceCam(float x, float y, float z)
{
	XMFLOAT3 center(x, y, z);
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);


	// 각 좌표축을 바라보는 사선 축.
	XMFLOAT3 target[6] = {

		XMFLOAT3(x + 1, y ,z), // +X
		XMFLOAT3(x - 1, y ,z), // -X
		XMFLOAT3(x, y + 1 ,z),  // +Y
		XMFLOAT3(x, y - 1 ,z), // -Y
		XMFLOAT3(x, y ,z + 1), // +Z
		XMFLOAT3(x, y ,z - 1) // -Z
	};



	XMFLOAT3 ups[6] = {
		XMFLOAT3(0, 1.0f , 0),
		XMFLOAT3(0, 1.0f , 0),
		XMFLOAT3(0, 0.0f , -1.0f),
		XMFLOAT3(0, 1.0f , 1.0f),
		XMFLOAT3(0, 1.0f , 0),
		XMFLOAT3(0, 1.0f , 0)
	};

//	XMFLOAT3(97.8f, 33.5f, 44.518f)

	for (int i = 0; i < 6; i++)
	{
		m_Cam[i].LookAt(center, target[i], ups[i]);
		m_Cam[i].SetProjection(0.5f * XM_PI, 1.0f , 0.1f, 1000.0f);
		m_Cam[i].UpdateViewMatrix();
	}



}

void cPointLightShadowMap::DarwDepthCubeMap(ID3D11Device * device, ID3D11DeviceContext * dc, sSkinModellInstance & skinModel)
{
	ID3D11RenderTargetView* RT[1];
	dc->RSSetViewports(6, m_ViewPort);

	dc->RSSetState(0);

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetInputLayout(InputLayOut::SkinInputLayout);

	ID3D11DepthStencilView* pDSV = m_DSV;
	
	// Clear the depth stencil
	
	RT[0] = nullptr;
	dc->OMSetRenderTargets(1, &RT[0], pDSV);
	dc->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	for (int i = 0; i < 6; i++)
	{
		//dc->ClearRenderTargetView(m_RTV[i], reinterpret_cast<const float*>(&Colors::Silver));
		dc->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH , 1.0, 0);
		DrawToCubeMap(skinModel, dc, &m_Cam[i], i, true);
	}

	dc->GenerateMips(m_SRV);

}

void cPointLightShadowMap::DrawToCubeMap(sSkinModellInstance & skinModel, ID3D11DeviceContext * dc, cCamera * cam, UINT count, BOOL hasAni)
{


	UINT stride = sizeof(sSkinnedVertex);
	UINT offset = 0;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX WVP;

	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX VP = XMMatrixMultiply(view, proj);


	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* smapTech;

	if (!hasAni)
		smapTech = EffectMGR::ShadowCubeMapE->GetTech("ShadowCubeMap");
	else   smapTech = EffectMGR::ShadowCubeMapE->GetTech("ShadowAniCubeMap");

	UINT meshCount = skinModel.Model->GetMeshCount();

	for (UINT meshID = 0; meshID < meshCount; meshID++)
	{
		smapTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; p++)
		{
			ID3D11Buffer* pIBuffer = skinModel.Model->GetIndiceBuffer(meshID);
			ID3D11Buffer* pVBuffer = skinModel.Model->GetVertexBuffer(meshID);

			dc->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
			dc->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R16_UINT, offset);

			world = XMLoadFloat4x4(&(skinModel.World));
			worldInvTranspose = MathHelper::InverseTranspose(world);
			WVP = world * view * proj;

			EffectMGR::ShadowCubeMapE->SetWorldViewProj(WVP);
			EffectMGR::ShadowCubeMapE->SetBoneTransform(&skinModel.FinalTransforms[0], skinModel.FinalTransforms.size());

			smapTech->GetPassByIndex(p)->Apply(0, dc);
			dc->DrawIndexed(skinModel.Model->GetIndiceBufferCount(meshID), 0, 0);
		}
		//}
	}



}
