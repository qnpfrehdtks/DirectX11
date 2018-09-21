#pragma once

#include "singletonBase.h"
#include "Vertex.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "SkinData.h"
#include <map>
#include <unordered_map>
#include "Bone.h"
#include "AnimationMGR.h"

struct sAnimationClip;
struct sBoneAni;
class cSkinnedAnimation;

using namespace Vertexs;

class AssimpFBXLoader : public SingletonBase<AssimpFBXLoader>
{
private:
	

private:
	static const UINT m_BoneMaxIdx = 4;

	std::unordered_map<std::string, aiNodeAnim*> m_NodeAniTable;
	std::unordered_map<std::string, int> m_BoneMappingTable;
	std::vector<aiNode*> m_NodeInfoTable;
	std::vector<sBone> m_BoneInfoTable;
	
	
	ID3D11Device * m_pDevice;
	const aiScene* m_pAiScene;
	Assimp::Importer m_Importer;

	INT m_BoneCount;

	UINT m_IndexCount = 0;
	UINT m_VerticeCount = 0;

	XMFLOAT4X4 m_Inverse;

public:
	AssimpFBXLoader();
	~AssimpFBXLoader();

	std::vector<sTEXTURE> m_Tex;
	void LoadFBXFile(
		std::string filename,
		std::string skinName,
		ID3D11Device * device,
		ID3D11DeviceContext * dc,
		std::vector<sSkinMesh> &meshes,
		cSkinnedAnimation* skinAni,
		std::vector<sTEXTURE>& texes,
		AnimationMGR* aniMGR);
	void BoneTransform(float Time, std::vector<XMFLOAT4X4>& FinalTransform/*, sAnimationClip& aniClip*/);

private:

	void CalcInterpolatedScaling(XMFLOAT3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(XMFLOAT4& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(XMFLOAT3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	
	UINT FindScalingAni(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindRotationAni(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindPositiongAni(float AnimationTime, const aiNodeAnim* pNodeAnim);



	void LoadMesh(aiMesh* mesh, aiNode* node, sSkinMesh& omesh, BOOL isSkinnedModel, AnimationMGR* aniMGR = nullptr);
	void LoadVertex(aiMesh* mesh, std::vector<sSkinnedVertex>& verticeVec, UINT vertexID);

	void LoadBone(aiMesh * mesh, aiNode* node, std::vector<sSkinnedVertex> &verticeVec, UINT BaseVertex);
	void LoadAni( aiAnimation* ani, sAnimationClip& aniClip);
	void LoadTextures(sSkinMesh& omesh, aiMesh * mesh, aiTextureType type);

	

	const aiNodeAnim* FindAniNode(const aiAnimation* pAnimation, const std::string NodeName);

	void LoadNode(aiNode* node, const aiScene* scene);
	

	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMFLOAT4X4& ParentTransform);
	
	void aiMatrixToXMMatrix(const aiMatrix4x4& source, XMFLOAT4X4& destMAt);
	void RightHandtoLeft( XMFLOAT4X4& source);

public:
	std::map<std::string, sAnimationClip>* GetAniTable();

};

