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

struct sBone
{
public:
	std::string boneName;
	XMFLOAT4X4 m_FinalMat;
	XMFLOAT4X4 m_Offset;
public:
	sBone() {};
	~sBone() {};
};


class AnimationMGR
{

private:
	float m_TicksPerSecond;
	float m_TimeInTicks;
	float m_Duration;

	float m_curAniMaxTime;

	const aiScene* m_Scene;
	aiNode * m_RootNode;

	XMFLOAT4X4 m_Inverse;
	UINT m_BoneMaxIdx;
	UINT m_BoneCount; 
	std::unordered_map<std::string, aiAnimation*> m_AniTable;
	std::unordered_map<std::string, aiNodeAnim*> m_NodeAniTable;
	std::unordered_map<std::string, int> m_BoneMappingTable;
	std::vector<aiNode*> m_NodeInfoTable;
	std::vector<sBone> m_BoneInfoTable;

	aiAnimation* m_curAnimation;

public:
	AnimationMGR();
	~AnimationMGR();


	void Init(const aiScene* aiScene);

	void CurAniSetting();
	
	void CalcInterpolatedScaling(XMFLOAT3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(XMFLOAT4& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(XMFLOAT3& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	float GetDuration() {return m_Duration;}
	float GetTimeInTicks() { return m_TimeInTicks; }
	float GetTicksPerSecond() { return m_TicksPerSecond; }

	UINT FindScalingAni(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindRotationAni(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindPositiongAni(float AnimationTime, const aiNodeAnim* pNodeAnim);

	void LoadBone(aiMesh * mesh, std::vector<sSkinnedVertex> &verticeVec);
	void LoadAni(aiAnimation* ani);

	const aiNodeAnim* FindAniNode(const aiAnimation* pAnimation, const std::string NodeName);

	void LoadNode(aiNode* node, const aiScene* scene);


	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMFLOAT4X4& ParentTransform, aiAnimation* anim);

	void aiMatrixToXMMatrix(const aiMatrix4x4& source, XMFLOAT4X4& destMAt);
	void RightHandtoLeft(XMFLOAT4X4& source);

	void BoneTransform(float* Time, std::vector<XMFLOAT4X4>& FinalTransform/*, sAnimationClip& aniClip*/);

	void SetTime(float time) { m_TimeInTicks = time; }

};

