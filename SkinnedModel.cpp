#include "DXUT.h"
#include "SkinnedModel.h"

#include "CustomFileLoader.h"
#include "FBXLoader.h"


void sSkinnedModelInstance::Update(float dt)
{
	TimePos += dt;
	Model->m_SkinData.GetFinalMatrix(ClipName, TimePos, FinalTransforms);

	//// 애니메이션 순회
	if (TimePos > Model->m_SkinData.GetCurClipEndTime(ClipName))
	{
    	TimePos = 0.0f;
	}
	
	
}
cSkinnedModel::cSkinnedModel()
{
}

cSkinnedModel::~cSkinnedModel()
{
}

//void cSkinnedModel::LoadCustomFile(ID3D11Device * device, const std::string & modelFileName, const std::wstring texturePath)
//{
//	/*std::vector<sCustomMaterial> MatArr;
//	cCustomFileLoader loader;
//	loader.LoadFile(modelFileName, m_VertexArr, m_IndiceArr, m_SubsetArr, MatArr, m_SkinData);
//
//	m_ModelMesh.SetIndices(device, &m_IndiceArr[0], m_IndiceArr.size());
//	m_ModelMesh.SetVertices(device, &m_VertexArr[0], m_VertexArr.size());
//	m_ModelMesh.SetSubsetTable(m_SubsetArr);
//
//	m_SubSetCount = MatArr.size();
//
//	for (int i = 0; i < m_SubSetCount; i++)
//	{
//		m_MatArr.push_back(MatArr[i].Mat);
//
//		//ID3D11ShaderResourceView* srv = TextureResourceMGR::GetInstance()->CreateSRV(device, texturePath + MatArr[i].DiffuseMapName);
//		//m_DiffuseSRVArr.push_back(srv);
//
//		//ID3D11ShaderResourceView* normalSRV = TextureResourceMGR::GetInstance()->CreateSRV(device, texturePath + MatArr[i].NormalMapName);
//		//m_NormalSRVArr.push_back(normalSRV);
//
//	}*/
//}

void cSkinnedModel::LoadCustsomFile(ID3D11Device * device, const std::string & modelFileName, const std::wstring texturePath)
{
	std::vector<sCustomMaterial> MatArr;
		cCustomFileLoader loader;
		loader.LoadFile(modelFileName, m_VertexArr, m_IndiceArr, m_SubsetArr, MatArr, m_SkinData);
	
		m_ModelMesh.SetIndices(device, &m_IndiceArr[0], m_IndiceArr.size());
		m_ModelMesh.SetVertices(device, &m_VertexArr[0], m_VertexArr.size());
		m_ModelMesh.SetMeshTable(m_SubsetArr);
	
		m_SubSetCount = MatArr.size();
	
		for (int i = 0; i < m_SubSetCount; i++)
		{
			m_MatArr.push_back(MatArr[i].Mat);
	
			ID3D11ShaderResourceView* srv = TextureResourceMGR::GetInstance()->CreateSRV(device, texturePath + MatArr[i].DiffuseMapName);
			m_DiffuseSRVArr.push_back(srv);
	
			//ID3D11ShaderResourceView* normalSRV = TextureResourceMGR::GetInstance()->CreateSRV(device, texturePath + MatArr[i].NormalMapName);
			//m_NormalSRVArr.push_back(normalSRV);
	
		}
}

void cSkinnedModel::LoadFbxFile(ID3D11Device * device, const std::string & modelFileName, const std::wstring texturePath)
{
	std::vector<sCustomMaterial> MatArr;
	FBXLoader loader;
	loader.InitFBXSDK();
	//loader.LoadFile(modelFileName, m_BaiscVertexArr, m_IndiceArr, m_SubsetArr, MatArr, m_SkinData);

//	m_ModelMesh.SetIndices(device, &m_IndiceArr[0], m_IndiceArr.size());
//	m_ModelMesh.SetVertices(device, &m_VertexArr[0], m_VertexArr.size());

}

void cSkinnedModel::LoadFbxAssimpFile(
	ID3D11Device * device, ID3D11DeviceContext* dc,
	const std::string & modelFileName, 
	const std::wstring texturePath)
{
		std::vector<sCustomMaterial> MatArr;
	FBXLoader loader;

		loader.InitAssimp(
			modelFileName,
			device,
			dc);

		for (int i = 0; i < 4; i++)
		{
			m_AssimpTextVertexArr[i] =loader.m_vertices[i];
			m_AssimpIndiceArr[i] = loader.m_indices[i];
		}


}

