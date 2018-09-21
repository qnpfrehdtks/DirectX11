#include "DXUT.h"
#include "BasicModel.h"
#include "CustomFileLoader.h"
#include "TextureResourceMGR.h"


BasicModel::BasicModel(ID3D11Device * device, const std::string & modelFilename, const std::wstring & texturePath)
{
	std::vector<sCustomMaterial> MatArr;
	cCustomFileLoader loader;
	loader.LoadFile(modelFilename, m_VertexArr, m_IndiceArr, m_SubsetArr, MatArr);

	m_ModelMesh.SetIndices(device, &m_IndiceArr[0], m_IndiceArr.size());
	m_ModelMesh.SetVertices(device, &m_VertexArr[0], m_VertexArr.size());
//	m_ModelMesh.SetSubsetTable(m_SubsetArr);

	m_SubSetCount = MatArr.size();

	for (int i = 0; i < m_SubSetCount; i++)
	{
		m_MatArr.push_back(MatArr[i].Mat);

		ID3D11ShaderResourceView* srv = TextureResourceMGR::GetInstance()->CreateSRV( device, texturePath + MatArr[i].DiffuseMapName);
		m_DiffuseSRVArr.push_back(srv);

		ID3D11ShaderResourceView* normalSRV = TextureResourceMGR::GetInstance()->CreateSRV(device, texturePath + MatArr[i].NormalMapName);
		m_NormalSRVArr.push_back(normalSRV);

	}


}

BasicModel::~BasicModel()
{
}
