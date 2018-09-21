#include "DXUT.h"
#include "cShape.h"
#include "MathHelper.h"
#include "Camera.h"
#include "LightMGR.h"
#include "Main.h"
#include "GeometryGenerator.h"
#include "ShadowMap.h"
#include "OutLineMap.h"
#include "RenderState.h"


#include <iostream>
#include <string>
#include <fstream>      // std::ifstream

using namespace Vertexs;
UINT cBasicShape::m_PtCount = 0;

using namespace DirectX;

cBasicShape::cBasicShape(ID3D11Device * device, ID3D11DeviceContext* dc) :
	m_pDevice(device), 
	m_pDeviceContext(dc), 
	m_TextureSRV(nullptr), 
	m_IB(nullptr), 
	m_VB(nullptr), 
	m_isRimLight(true), m_fProgress(1.0f) , m_fEdge(0.01f), m_CurTime(0.0f)
{


	m_IdxCount = m_PtCount;
	m_RimLightColor = XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f);

	m_Color = XMFLOAT4(
		MathHelper::RandF(0, 80), 
		MathHelper::RandF(0, 80),
		MathHelper::RandF(0, 80)
		, 1.0f);

	// Shape 의 위치 설정.
	//m_Pos = XMFLOAT3(9.0f - (m_PtCount * 2.0f), 0.5f + ( m_PtCount * 0.5f) , 0.0f);
	//m_Pos = XMFLOAT3(5.0f - (m_PtCount * 5.0f), 1.0f, 0.0f);
	m_Pos = XMFLOAT3(MathHelper::RandF(-270.0f, 270.0f), MathHelper::RandF(4.0f, 250.0f), MathHelper::RandF(-270.0f, 270.0f));
	XMFLOAT3 ptLightPos = XMFLOAT3
	(
		m_Pos.x,
		m_Pos.y,
		m_Pos.z);

	m_Scale = XMFLOAT3(0.9f, 0.9f, 0.9f);

	// Shape에 장착할 PointLight 장착.
	sPtLight pptLight = LightMGR::GetInstance()->CreatePointLight
	(
		m_Pos,
		m_Color,
		m_Color,
		XMFLOAT4(m_Color.x *2, m_Color.y * 2, m_Color.z * 2,1.0f),
			30, XMFLOAT3(MathHelper::RandF(0.2f, 0.4f), MathHelper::RandF(0.2f, 0.4f), MathHelper::RandF(0.2f, 0.4f))
	);

	m_PtLight = pptLight;

	
	XMStoreFloat4x4(&m_World,  XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *    XMMatrixTranslation(m_Pos.x , m_Pos.y, m_Pos.z));
	//XMStoreFloat4x4(&m_World, XMMatrixIdentity());

	m_Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Material.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_Material.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 999);
	m_Material.Reflect = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);

	m_PtCount++;

	//InitAnimation();

}



cBasicShape::~cBasicShape()
{
	
}

void cBasicShape::SettingWorld()
{
	XMStoreFloat4x4(&m_World,XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * XMMatrixTranslation(m_Pos.x, m_Pos.y, m_Pos.z));
}

void cBasicShape::SetNormalBuffer()
{
	m_Stride = sizeof(sNormalVertex);
	GeometryGenerator geo;
	GeometryGenerator::MeshData shape;
	//geo.CreateSphere(2.0f, 16, 16, shape);
	geo.CreateBox(1.0f, 1.0f, 1.0f, shape);
	m_IndexCount = shape.Indices.size();
	m_VertexCount = shape.Vertices.size();

	std::vector<sNormalVertex> vertices(m_VertexCount);
	for (int i = 0; i < m_VertexCount; i++)
	{
		vertices[i].Pos = shape.Vertices[i].Position;
		vertices[i].Normal = shape.Vertices[i].Normal;
		vertices[i].Tex = shape.Vertices[i].TexC;
		vertices[i].TangentU = shape.Vertices[i].TangentU;

	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sNormalVertex) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), shape.Indices.begin(), shape.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) *m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));

}

void cBasicShape::SetBaiscBuffer(GeometryGenerator::MeshData* shape)
{
	m_Stride = sizeof(sBasicVertex);
//	GeometryGenerator geo;
//	GeometryGenerator::MeshData shape;
	//geo.CreateBox(1, 1, 1, shape);
	//geo.CreateCylinder(0.5f, 0.5f, 1.0f, 15, 15, shape);
//	geo.CreateGrid(20, 20,32,32, shape);

	m_IndexCount = (*shape).Indices.size();
	m_VertexCount = (*shape).Vertices.size();

	std::vector<sBasicVertex> vertices(m_VertexCount);
	for (int i = 0; i < m_VertexCount; i++)
	{
		vertices[i].Pos = (*shape).Vertices[i].Position;
		vertices[i].Normal = (*shape).Vertices[i].Normal;
		vertices[i].Tex = (*shape).Vertices[i].TexC;

	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sBasicVertex) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), (*shape).Indices.begin(), (*shape).Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) *m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));


}

void cBasicShape::SetBaiscBuffer(std::vector<sSkinnedVertex>& vec, std::vector<USHORT>& idxVec)
{

	m_Stride = sizeof(sBasicVertex);
	//	GeometryGenerator geo;
	//	GeometryGenerator::MeshData shape;
	//geo.CreateBox(1, 1, 1, shape);
	//geo.CreateCylinder(0.5f, 0.5f, 1.0f, 15, 15, shape);
	//	geo.CreateGrid(20, 20,32,32, shape);

	m_IndexCount = idxVec.size();
	m_VertexCount = vec.size();
	//m_VertexCount = (*shape).Vertices.size();

	std::vector<sBasicVertex> vertices(m_VertexCount);
	for (int i = 0; i < m_VertexCount; i++)
	{
		vertices[i].Pos = vec[i].Pos;
		vertices[i].Normal= vec[i].Normal;
		vertices[i].Tex = vec[i].Tex;
		//vertices[i].Tex = (*shape).Vertices[i].TexC;

	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sBasicVertex) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), idxVec.begin(), idxVec.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) *m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));

}

void cBasicShape::SetDissolveBuffer()
{
	m_Stride = sizeof(sDissolveVertex);
	GeometryGenerator geo;
	GeometryGenerator::MeshData shape;
	geo.CreateSphere(0.3f, 16, 16, shape);

	m_IndexCount = shape.Indices.size();
	m_VertexCount = shape.Vertices.size();

	std::vector<sDissolveVertex> vertices(m_VertexCount);
	for (int i = 0; i < m_VertexCount; i++)
	{
		vertices[i].Pos = shape.Vertices[i].Position;
		vertices[i].Color = m_Color;
		vertices[i].Tex = shape.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sDissolveVertex) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), shape.Indices.begin(), shape.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) *m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));
}

void cBasicShape::SetSimpleBuffer()
{
	m_Stride = sizeof(sSimple);
	GeometryGenerator geo;
	GeometryGenerator::MeshData shape;
	geo.CreateSphere(0.3f, 16, 16, shape);

	m_IndexCount = shape.Indices.size();
	m_VertexCount = shape.Vertices.size();

	std::vector<sSimple> vertices(m_VertexCount);
	for (int i = 0; i < m_VertexCount; i++)
	{
		vertices[i].Pos = shape.Vertices[i].Position;
		vertices[i].Color = m_Color;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(sSimple) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	std::vector<UINT> indices;
	indices.insert(indices.end(), shape.Indices.begin(), shape.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) *m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));
}

void cBasicShape::SetSkullBuffer()
{
	m_Stride = sizeof(sBasicVertex);
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> m_VertexCount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;


	std::vector<Vertexs::sBasicVertex> vertices(m_VertexCount);

	for (UINT i = 0; i < m_VertexCount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		vertices[i].Tex.x = 0;
		vertices[i].Tex.y = 0;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_IndexCount = 3 * tcount;
	std::vector<UINT> indices(m_IndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertexs::sBasicVertex) * m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));


}

void cBasicShape::GbufferDraw(cShadowMap * pShadowMap, cOutLineMap * pOutLineMap)
{
//	sPtLight ptLights[3];
//	ptLights[0] = (LightMGR::GetInstance()->GetPtLights()[0].get()->ptLight);
	//	ptLights[1] = (LightMGR::GetInstance()->GetPtLights()[1].get()->ptLight);
	//ptLights[2] = (LightMGR::GetInstance()->GetPtLights()[2].get()->ptLight);

	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	//const sDirectionalLight* dirLight = LightMGR::GetInstance()->GetDirLights()[0].get();

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	XMMATRIX T = XMMATRIX(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_VB, &m_Stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, offset);

	// Set constants
	XMMATRIX W = XMLoadFloat4x4(&m_World);
	XMMATRIX V = cam->GetViewMat();
	XMMATRIX P = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(W);
	XMMATRIX worldInvTransposeView = worldInvTranspose * V;
	XMMATRIX WV = W * V;
	XMMATRIX WVP = WV * P;
	XMMATRIX VP = V * P;
	// ===================== World, Proj, View... ===================
	EffectMGR::gBufferE->SetWorld(W);
//	EffectMGR::gBufferE->SetViewProj(VP);
//	EffectMGR::gBufferE->SetWorldView(WV);
	EffectMGR::gBufferE->SetWorldViewProj(WVP);
//	EffectMGR::gBufferE->SetWorldViewProjTex(V * P * T);
//	EffectMGR::gBufferE->SetWorldInvTransposeView(worldInvTransposeView);
	EffectMGR::gBufferE->SetWorldInvTranspose(worldInvTranspose);
//	EffectMGR::gBufferE->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

	EffectMGR::gBufferE->SetDiffuseMap(m_TextureSRV);

	//  ======================= Light ==========================
//	EffectMGR::gBufferE->SetDirLights(&(dirLight->dirLight));
//	EffectMGR::gBufferE->SetPtLights(ptLights, 1);

	EffectMGR::gBufferE->SetEyePosW(cam->GetPos());

	//EffectMGR::gBufferE->SetTexTransform(XMMatrixIdentity());
//	EffectMGR::gBufferE->SetMaterial(m_Material);

	// Shadow 영향을 받을 것 인가 아닌가?
	if (pShadowMap != nullptr)
	{
		XMMATRIX shadowTransform = XMLoadFloat4x4(&pShadowMap->GetShadowTransform());
	//	EffectMGR::gBufferE->Set(pShadowMap->GetShadowMap());
		//EffectMGR::gBufferE->SetShadowTransform(world * shadowTransform);
	}
	// OutLine , Normal Map에 영향을 받을것인가?
	if (pOutLineMap != nullptr)
	{
		XMMATRIX outLineTransform = XMLoadFloat4x4(&pOutLineMap->GetOutLineTransform());
	//	EffectMGR::gBufferE->SetOutLine(pOutLineMap->GetOutLineSRVMap());
	//	EffectMGR::gBufferE->SetOutLineTransform(outLineTransform);
	}
	// =================== Rim Light ====================
	//EffectMGR::gBufferE->SetRimColor(m_RimLightColor);
	//EffectMGR::gBufferE->SetRimWidth(0.5f);

	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* tech;

	if (m_IdxCount <= 1)
		tech = EffectMGR::gBufferE->GetTech("GbufferBasic");
	else
	{
		tech = EffectMGR::gBufferE->GetTech("GbufferBasic");
	}

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
	}
}

void cBasicShape::Draw(cShadowMap* pShadowMap, cOutLineMap* pOutLineMap)
{
	sPtLight ptLights[3];
	ptLights[0] = (*LightMGR::GetInstance()->GetPtLights())[0];
//	ptLights[1] = (LightMGR::GetInstance()->GetPtLights()[1].get()->ptLight);
	//ptLights[2] = (LightMGR::GetInstance()->GetPtLights()[2].get()->ptLight);

	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	const sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;

	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_VB, &m_Stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, offset);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldInvTransposeViewProj = worldInvTranspose * view * proj;
	XMMATRIX worldViewProj = world * view * proj;

	// ===================== World, Proj, View... ===================
	EffectMGR::BasicE->SetWorld(world);
	EffectMGR::BasicE->SetViewProj(view * proj);
	EffectMGR::BasicE->SetWorldViewProj(worldViewProj);
	EffectMGR::BasicE->SetWorldViewProjTex(worldViewProj * toTexSpace);
//	EffectMGR::BasicE->SetWorldInvTransposeViewProj(worldInvTransposeViewProj);
	EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);

	EffectMGR::BasicE->SetDiffuseMap(m_TextureSRV);

	//  ======================= Light ==========================
	EffectMGR::BasicE->SetDirLights(&dirLight);
	EffectMGR::BasicE->SetPtLights(&ptLights[0], 1);

	EffectMGR::BasicE->SetEyePosW(cam->GetPos());

	EffectMGR::BasicE->SetTexTransform(XMMatrixIdentity());
	EffectMGR::BasicE->SetMaterial(m_Material);

	// Shadow 영향을 받을 것 인가 아닌가?
	if (pShadowMap != nullptr)
	{
		XMMATRIX shadowTransform = XMLoadFloat4x4(&pShadowMap->GetShadowTransform());
		EffectMGR::BasicE->SetShadowMap(pShadowMap->GetShadowMap());
		EffectMGR::BasicE->SetShadowTransform(world * shadowTransform);
	}
	// OutLine , Normal Map에 영향을 받을것인가?
	if (pOutLineMap != nullptr)
	{
		XMMATRIX outLineTransform = XMLoadFloat4x4(&pOutLineMap->GetOutLineTransform());
		EffectMGR::BasicE->SetOutLine(pOutLineMap->GetOutLineSRVMap());
		EffectMGR::BasicE->SetOutLineTransform(outLineTransform);
	}
	// =================== Rim Light ====================
	EffectMGR::BasicE->SetRimColor(m_RimLightColor);
	EffectMGR::BasicE->SetRimWidth(0.5f);

	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* tech;

	if (m_IdxCount <= 1)
		tech = EffectMGR::BasicE->GetTech("TexDirLight");
	else
	{
		tech = EffectMGR::BasicE->GetTech("TexDirLight");
	}

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
	}
}

void cBasicShape::DissolveDraw(cShadowMap * pShadowMap, cOutLineMap * pOutLineMap,
	ID3D11ShaderResourceView* DissolveSRV, ID3D11ShaderResourceView* DissolveRampSRV)
{
	//sPtLight* ptLights[3];
	//ptLights[0] = (LightMGR::GetInstance()->GetPtLights()[0]);
	//ptLights[1] = (LightMGR::GetInstance()->GetPtLights()[1]);
	//ptLights[2] = (LightMGR::GetInstance()->GetPtLights()[2]);

	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	const sDirLight dirLight = (*LightMGR::GetInstance()->GetDirLights())[0];

	UINT stride = sizeof(sBasicVertex);
	UINT offset = 0;


	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(InputLayOut::BasicInputLayout);

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_VB, &m_Stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, offset);

	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);


	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldInvTransposeViewProj = worldInvTranspose * view * proj;
	XMMATRIX worldViewProj = world * view * proj;

	// ===================== World, Proj, View... ===================
	EffectMGR::BasicE->SetWorld(world);
	EffectMGR::BasicE->SetViewProj(view * proj);
	EffectMGR::BasicE->SetWorldViewProj(worldViewProj);
	EffectMGR::BasicE->SetWorldViewProjTex(worldViewProj * toTexSpace);
//	EffectMGR::BasicE->SetWorldInvTransposeViewProj(worldInvTransposeViewProj);
	EffectMGR::BasicE->SetWorldInvTranspose(worldInvTranspose);

	EffectMGR::BasicE->SetDiffuseMap(m_TextureSRV);


	// =================== Dissolve ============================
	EffectMGR::BasicE->SetProgress(m_fProgress);
	EffectMGR::BasicE->SetEdge(m_fEdge);

	EffectMGR::BasicE->SetEdgeRange(0.2f);
	EffectMGR::BasicE->SetDissolveMap(DissolveSRV);
	EffectMGR::BasicE->SetDissolveColorMap(DissolveRampSRV);

	//  ======================= Light ==========================
	EffectMGR::BasicE->SetDirLights((&dirLight));
//	EffectMGR::BasicE->SetPtLights(&ptLights[0], 3);

	EffectMGR::BasicE->SetEyePosW(cam->GetPos());

	EffectMGR::BasicE->SetTexTransform(XMMatrixIdentity());
	EffectMGR::BasicE->SetMaterial(m_Material);

	// Shadow 영향을 받을 것 인가 아닌가?
	if (pShadowMap != nullptr)
	{
		XMMATRIX shadowTransform = XMLoadFloat4x4(&pShadowMap->GetShadowTransform());
		EffectMGR::BasicE->SetShadowMap(pShadowMap->GetShadowMap());
		EffectMGR::BasicE->SetShadowTransform(world * shadowTransform);
	}
	// OutLine , Normal Map에 영향을 받을것인가?
	if (pOutLineMap != nullptr)
	{
		XMMATRIX outLineTransform = XMLoadFloat4x4(&pOutLineMap->GetOutLineTransform());
		EffectMGR::BasicE->SetOutLine(pOutLineMap->GetOutLineSRVMap());
		EffectMGR::BasicE->SetOutLineTransform(outLineTransform);
	}

	// =================== Rim Light ====================
	EffectMGR::BasicE->SetRimColor(m_RimLightColor);
	EffectMGR::BasicE->SetRimWidth(0.8f);

	D3DX11_TECHNIQUE_DESC techDesc;

	ID3DX11EffectTechnique* tech;
	tech = EffectMGR::BasicE->GetTech("BasicDissolve");

	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_pDeviceContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
	}

	m_pDeviceContext->RSSetState(0);
//	m_pDeviceContext->OMSetBlendState(0, 0, 0);

}

void cBasicShape::InitAnimation()
{
	XMVECTOR q0 = XMQuaternionRotationAxis(XMVectorSet(0.0f, MathHelper::RandF(0.0f, 1.0f), 0.0f, 0.0f), XMConvertToRadians(30.0f));
	XMVECTOR q1 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 1.0f, MathHelper::RandF(1.0f, 2.0f), 0.0f), XMConvertToRadians(45.0f));
	XMVECTOR q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, MathHelper::RandF(0.5f, 1.0f), 0.0f, 0.0f), XMConvertToRadians(-30.0f));
	XMVECTOR q3 = XMQuaternionRotationAxis(XMVectorSet(1.0f, MathHelper::RandF(0.0f, 1.0f), 0.0f, 0.0f), XMConvertToRadians(70.0f));

	m_ShapeAnimation.Keyframes.resize(5);
	m_ShapeAnimation.Keyframes[0].TimePos = 0.0f;
	m_ShapeAnimation.Keyframes[0].Translation = XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z);
	m_ShapeAnimation.Keyframes[0].Scale = XMFLOAT3(m_Scale.x, m_Scale.y, m_Scale.z);
	XMStoreFloat4(&m_ShapeAnimation.Keyframes[0].Quaternion, q0);

	m_ShapeAnimation.Keyframes[1].TimePos = 2.0f;
	m_ShapeAnimation.Keyframes[1].Translation = XMFLOAT3(0.0f, MathHelper::RandF(4.0f, 7.0f), MathHelper::RandF(3.0f, 7.0f));
	m_ShapeAnimation.Keyframes[1].Scale = XMFLOAT3(m_Scale.x, m_Scale.y, m_Scale.z);
	XMStoreFloat4(&m_ShapeAnimation.Keyframes[1].Quaternion, q1);

	m_ShapeAnimation.Keyframes[2].TimePos = 4.0f;
	m_ShapeAnimation.Keyframes[2].Translation = XMFLOAT3(MathHelper::RandF(3.0f, 7.0f), MathHelper::RandF(3.0f, 6.0f), MathHelper::RandF(3.0f, 6.0f));
	m_ShapeAnimation.Keyframes[2].Scale = XMFLOAT3(m_Scale.x, m_Scale.y, m_Scale.z);
	XMStoreFloat4(&m_ShapeAnimation.Keyframes[2].Quaternion, q2);

	m_ShapeAnimation.Keyframes[3].TimePos = 6.0f;
	m_ShapeAnimation.Keyframes[3].Translation = XMFLOAT3(0.0f, 3.0f, -5.0f);
	m_ShapeAnimation.Keyframes[3].Scale = XMFLOAT3(m_Scale.x, m_Scale.y, m_Scale.z);
	XMStoreFloat4(&m_ShapeAnimation.Keyframes[3].Quaternion, q3);

	m_ShapeAnimation.Keyframes[4].TimePos = 8.0f;
	m_ShapeAnimation.Keyframes[4].Translation = XMFLOAT3(m_Pos.x, m_Pos.y, m_Pos.z);
	m_ShapeAnimation.Keyframes[4].Scale = XMFLOAT3(m_Scale.x, m_Scale.y, m_Scale.z);
	XMStoreFloat4(&m_ShapeAnimation.Keyframes[4].Quaternion, q0);




}

void cBasicShape::UpdateDissolve()
{
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		if (m_fProgress < 0.0f)
		{
			m_fProgress = 0;
			return;
		}

		m_fProgress -= 0.01f;
	}
	else if (GetAsyncKeyState('E') & 0x8000)
	{
		if (m_fProgress >= 1.0f)
		{
			m_fProgress = 1.0f;
			return;
		}

		m_fProgress += 0.01f;
	}
	else if (GetAsyncKeyState('Z') & 0x8000)
	{
		if (m_fEdge <= 0.001f)
		{
			m_fEdge = 0.001f;
			return;
		}

		m_fEdge -= 0.001f;
	}
	else if (GetAsyncKeyState('X') & 0x8000)
	{

		if (m_fEdge >= 0.5f)
		{
			m_fEdge = 0.5f;
			return;
		}

		m_fEdge += 0.001f;
	}
}

void cBasicShape::Update(float dt)
{
	m_CurTime += dt;

	if (m_CurTime >= m_ShapeAnimation.GetEndTime()) m_CurTime = 0.0f;


	m_ShapeAnimation.Interpolate(m_CurTime, m_World);
	//m_PtLight->Position.x = m_World._41;
	//m_PtLight->Position.y = m_World._42;
	//m_PtLight->Position.z = m_World._43;
}

cSimpleShape::cSimpleShape(ID3D11Device* device, ID3D11DeviceContext* dc) : cBasicShape(device,dc)
{

}

cSimpleShape::~cSimpleShape()
{
}

void cSimpleShape::Draw()
{
	const cCamera* cam = D3DMain::GetInstance()->GetCam();
	UINT offset = 0;


	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_VB, &m_Stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_IB, DXGI_FORMAT_R32_UINT, offset);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX view = cam->GetViewMat();
	XMMATRIX proj = cam->GetProjMat();
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	EffectMGR::SimpleE->SetWorld(world);
	EffectMGR::SimpleE->SetWorldViewProj(worldViewProj);

	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* tech = EffectMGR::SimpleE->GetTech("Light1");
	tech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
	}
}

cNormalShape::cNormalShape(ID3D11Device * device, ID3D11DeviceContext * dc) : cBasicShape(device, dc)
{
}

cNormalShape::~cNormalShape()
{
}

void cNormalShape::Draw()
{


}
