#ifndef SKINNEDMODEL_H
#define SKINNEDMODEL_H
#include "SkinnedData.h"
#include "MeshGeometry.h"
#include "TextureResourceMGR.h"
#include "Vertex.h"
#include "DXUT.h"
#include "LightMGR.h"

using namespace Vertexs;

class cSkinnedModel
{
public:
	cSkinnedModel();
	~cSkinnedModel();

	void LoadCustsomFile(ID3D11Device* device, const std::string& modelFileName, const std::wstring texturePath);
	void LoadFbxFile(ID3D11Device* device, const std::string& modelFileName, const std::wstring texturePath);
	void LoadFbxAssimpFile(ID3D11Device* device, ID3D11DeviceContext* dc, const std::string& modelFileName, const std::wstring texturePath);

	UINT m_SubSetCount;

	std::vector<sMaterial> m_MatArr;
	std::vector<ID3D11ShaderResourceView*> m_DiffuseSRVArr;
	std::vector<ID3D11ShaderResourceView*> m_NormalSRVArr;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<sSkinnedVertex> m_VertexArr;
	std::vector<sBasicVertex> m_BaiscVertexArr;
	std::vector<USHORT> m_IndiceArr;
	std::vector<MeshGeometry::sMeshSet> m_SubsetArr;

	std::vector<sBasicVertex> m_AssimpTextVertexArr[4];
	std::vector<USHORT> m_AssimpIndiceArr[4];

	MeshGeometry m_ModelMesh;
	cSkinnedData m_SkinData;



};

struct sSkinnedModelInstance
{
	cSkinnedModel* Model;
	float TimePos;
	std::string ClipName;
	XMFLOAT4X4 World;
	std::vector<XMFLOAT4X4> FinalTransforms;

	void Update(float dt);
};



#endif // SKINNEDMODEL_H

