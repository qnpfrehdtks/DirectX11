#pragma once
#include "DXUT.h"
#include <vector>

using namespace DirectX;

class MeshGeometry
{
public:
	struct sMeshSet
	{
		UINT ID;
		UINT VertexStart;
		UINT VertexCount;
		UINT FaceStart;
		UINT FaceCount;

		UINT IndiceCount;

		sMeshSet() : ID(-1), IndiceCount(0)
		{

		}

	};
	MeshGeometry();
	~MeshGeometry();

	template <typename VertexType>
	void SetVertices(ID3D11Device* device, const VertexType* vertices, UINT count);
	void SetIndices(ID3D11Device* device, const USHORT* indices, UINT count);

	void Draw(ID3D11DeviceContext* dc, UINT subsetId);

public:
	ID3D11Buffer * m_VB;
	ID3D11Buffer* m_IB;


	void SetMeshTable(std::vector<sMeshSet>& subsetTable)
	{
		m_SubMeshSet = subsetTable;
	}

	private:
	

		UINT m_VertexStride;

		std::vector<sMeshSet> m_SubMeshSet;


};

template<typename VertexType>
 void MeshGeometry::SetVertices(ID3D11Device* device, const VertexType * vertices, UINT count)
{
	 m_VertexStride = sizeof(VertexType);


	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = m_VertexStride * count;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;

	HR(device->CreateBuffer(&vbd, &vinitData, &m_VB));
}
