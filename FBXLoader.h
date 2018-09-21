#pragma once
#include <fbxsdk.h>
#include "Vertex.h"
#include "MeshGeometry.h"
#include "SkinnedData.h"
#include "CustomFileLoader.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace Vertexs;


class FBXLoader : public SingletonBase<FBXLoader>
{
	UINT m_IdxCount;
	FbxManager* m_pFbxMgr;
	FbxScene* m_pFbxScene;

	const aiScene* m_pAiScene;
public:
	std::vector<sBasicVertex> m_vertices[4];
	std::vector<USHORT> m_indices[4];


	FBXLoader();
	~FBXLoader();

	void InitFBXSDK();
	void InitAssimp(
		std::string filename, 
		ID3D11Device* device, 
		ID3D11DeviceContext* dc);

	void ProcessAssimp(aiNode* node, const aiScene * scene);
	void ProcessMesh(
		aiMesh* mesh, 
		const aiScene* scene, 
		std::vector<sBasicVertex>& vertices,
		std::vector<USHORT>& indices );


	void LoadFile(const std::string filename,
		std::vector<sBasicVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::sMeshSet>& subsets,
		std::vector<sCustomMaterial>& mats,
		cSkinnedData& skinInfo
	);

	void LoadFBX(FbxNode* node, std::vector<sBasicVertex>& vertexBuffer, std::vector<USHORT>& idxBuffer);
	XMFLOAT3 LoadNormal(FbxMesh* mesh, int controlPointIndex, int vertexCount);
	XMFLOAT2 LoadUV(FbxMesh* mesh, int controlPointIndex, int vertexCount);


};

