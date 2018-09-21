#include "DXUT.h"
#include "Sky.h"
#include "EffectMGR.h"
#include "GeometryGenerator.h"
#include"Vertex.h"
#include "Camera.h"
#include "DDSTextureLoader.h"

using namespace Vertexs;


cSky::cSky(ID3D11Device * device, const std::wstring & cubemapFilename, float SkyRadius)
{
	HR(CreateDDSTextureFromFile(device, cubemapFilename.c_str(), nullptr, &m_SkySRV));

	BuildSkyGeometry(device, SkyRadius);

}

cSky::~cSky()
{
}

void cSky::BuildSkyGeometry(ID3D11Device * device, float SkyRadius)
{

	GeometryGenerator geo;
	GeometryGenerator::MeshData skyHemi;
	geo.CreateSphere(SkyRadius, 124, 124, skyHemi);

	m_IdexCount = skyHemi.Indices.size();
	UINT verCount = skyHemi.Vertices.size();

	std::vector<XMFLOAT3> vertices(skyHemi.Vertices.size());

	for (int i = 0; i < verCount; i++)
	{
		vertices[i] = skyHemi.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = sizeof(sPos) * verCount;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	HR(device->CreateBuffer(&vbd, &vinitData, &m_skyVB));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * m_IdexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	std::vector<USHORT> indices16;
	indices16.assign(skyHemi.Indices.begin(), skyHemi.Indices.end());

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices16[0];

	HR(device->CreateBuffer(&ibd, &iinitData, &m_skyIB));


}

void cSky::DrawSky(ID3D11DeviceContext * dc, const cCamera* camera)
{
	UINT stride = sizeof(sPos);
	UINT offset = 0;

	auto eyePos = camera->GetPos();
	auto view = camera->GetViewMat();
	auto proj = camera->GetProjMat();
	auto world = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	auto WVP = world * view * proj;

	EffectMGR::SkyE->SetWorldViewProj(WVP);
	EffectMGR::SkyE->SetCubeMap(m_SkySRV);

	dc->IASetInputLayout(InputLayOut::PosInputLayout);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetIndexBuffer(m_skyIB, DXGI_FORMAT_R16_UINT, offset);
	dc->IASetVertexBuffers(0, 1, &m_skyVB, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	auto tech = EffectMGR::SkyE->GetTech("SkyRender");

	tech->GetDesc(&techDesc);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(m_IdexCount, 0, 0);
	}



}

void cSky::DrawGbufferSky(ID3D11DeviceContext * dc, const cCamera * camera)
{
	UINT stride = sizeof(sPos);
	UINT offset = 0;

	auto eyePos = camera->GetPos();
	auto view = camera->GetViewMat();
	auto proj = camera->GetProjMat();
	auto world = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	auto WVP = world * view * proj;

	EffectMGR::gBufferE->SetWorldViewProj(WVP);
	EffectMGR::gBufferE->SetCubeMap(m_SkySRV);

	dc->IASetInputLayout(InputLayOut::PosInputLayout);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetIndexBuffer(m_skyIB, DXGI_FORMAT_R16_UINT, offset);
	dc->IASetVertexBuffers(0, 1, &m_skyVB, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	auto tech = EffectMGR::gBufferE->GetTech("SkyRender");

	tech->GetDesc(&techDesc);

	for (int p = 0; p < techDesc.Passes; p++)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(m_IdexCount, 0, 0);
	}

}
