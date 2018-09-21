#include "DXUT.h"
#include "MeshGeometry.h"


MeshGeometry::MeshGeometry()
{
}


MeshGeometry::~MeshGeometry()
{
}

void MeshGeometry::SetIndices(ID3D11Device * device, const USHORT * indices, UINT count)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &iinitData, &m_IB));
}

void MeshGeometry::Draw(ID3D11DeviceContext * dc, UINT subsetId)
{
	UINT offset = 0;

	dc->IASetVertexBuffers(0, 1, &m_VB, &m_VertexStride, &offset);
	dc->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);

	dc->DrawIndexed(
		m_SubMeshSet[subsetId].FaceCount * 3,
		m_SubMeshSet[subsetId].FaceStart * 3,
		0);
}
