#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "DXUT.h"
#include "MeshGeometry.h"
#include "TextureResourceMGR.h"
#include "Vertex.h"
#include "LightMGR.h"

using namespace Vertexs;

class BasicModel
{
public:
	BasicModel(ID3D11Device* device, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicModel();

	UINT m_SubSetCount;

	std::vector<sMaterial> m_MatArr;
	std::vector<ID3D11ShaderResourceView*> m_DiffuseSRVArr;
	std::vector<ID3D11ShaderResourceView*> m_NormalSRVArr;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<sNormalVertex> m_VertexArr;
	std::vector<USHORT> m_IndiceArr;
	std::vector<MeshGeometry::sMeshSet> m_SubsetArr;

	MeshGeometry m_ModelMesh;
};

struct BasicModelInstance
{
	BasicModel* Model;
	XMFLOAT4X4 World;
};

#endif // SKINNEDMODEL_H
