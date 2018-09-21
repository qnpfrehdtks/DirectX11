#ifndef MESH_H
#define MESH_H

#include "DXUT.h"
#include <vector>
#include "Vertex.h"
#include "LightMGR.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace DirectX;
using namespace Vertexs;



struct sTEXTURE
{
	ID3D11ShaderResourceView* m_RSV;
	std::string fileName;
	aiTextureType type;
};


class sSkinMesh
{

public:
	sSkinMesh() : m_IndexCount(0), m_VerticeCount(0), m_BaseVertex(0), m_BaseIndex(0)
	{};

private:
	USHORT m_MaterialIdx;
	
	UINT m_IndexCount;
	UINT m_VerticeCount;
	UINT m_BaseVertex;
	UINT m_BaseIndex;

	UINT m_VertexStride;
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

	ID3D11Device* m_pDevice;

	std::vector<sSkinnedVertex> m_Vertices;
	std::vector<sMaterial> m_Mat;
	std::vector<USHORT> m_Indices;
	//sTEXTURE m_Texture;
	std::vector<sTEXTURE> m_DiffuseTexture;
	std::vector<sTEXTURE> m_Textures;

public:

	void Init(
		ID3D11Device* device, 
		std::vector<sSkinnedVertex>& vertices,
		std::vector<USHORT>& indices, USHORT matIdx
	)
	{
		m_MaterialIdx = matIdx;
		m_IndexCount = indices.size();
		m_VerticeCount = vertices.size();

		m_pDevice = device;
		m_Vertices = vertices;
		m_Indices = indices;
	}
	~sSkinMesh() { int a = 0; }

	// ========================================
	//                  SET
	// ========================================
	template<typename VertexType>
	void SetVertices(const VertexType * vertices, UINT count);
	void SetIndices(const USHORT* indices, UINT count);

	void SetBaseVertexSize(UINT size) { m_BaseVertex = size; }
	void SetBaseIndexSize(UINT size) { m_BaseIndex = size; }

	// ========================================
	//                  GET
	// ========================================
	ID3D11Buffer* GetpVertexBuffer() { return m_VB; }
	ID3D11Buffer* GetpIndiceBuffer() { return m_IB; }

	std::vector<sSkinnedVertex>& GetVertices() { return m_Vertices; }
	std::vector<USHORT>& GetIndices() { return m_Indices; }

	sTEXTURE* GetTex(int idx) { return &m_Textures[idx]; }

	UINT GetIndiceCount() { return m_IndexCount; }
	UINT GeTexCount() { return m_Textures.size(); }
	// ========================================
	//                  ETC
	// ========================================
	void Draw(ID3D11DeviceContext* dc, UINT stride);
	
	void InsertTexture(std::string filename, aiTextureType type);
	
};


template<typename VertexType>
void sSkinMesh::SetVertices(const VertexType * vertices, UINT count)
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

	HR(m_pDevice->CreateBuffer(&vbd, &vinitData, &m_VB));
}

#endif

