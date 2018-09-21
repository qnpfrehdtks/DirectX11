#pragma once
#include "DXUT.h"
#include <vector>
#include <map>


// 참고. Frank D Luna Animation Part;
using namespace DirectX;


// 각 Key 당 위치와 쿼터니언, 크기를 저장하기 위한 Struct
struct sKeyFrame
{

	sKeyFrame();
	~sKeyFrame();

	float TimePos;
	XMFLOAT3 Scale;
	XMFLOAT3 Translation;
	XMFLOAT4 Quaternion;
};

// 각 Key Frame을 가진 애니메이션을 정의한 구조체.
struct sBoneAnimation
{
private:
//	float curTime;

public:
	sBoneAnimation();
	~sBoneAnimation();

	float GetStartTime()const { return Keyframes.front().TimePos; }
	float GetEndTime()const { return Keyframes.back().TimePos; }

	void Interpolate(float dt, XMFLOAT4X4& M);
	std::vector<sKeyFrame> Keyframes;

};

// 개별 동작을 구성하는 뼈대별 애니메이션들의 집합을 
// 애니메이션 Clip 이라 지칭한다. 
// 그렇기 때문에 뼈대 애니메이션 배열을 두고 다님.
struct sAnimationClips
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;

	void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransform);

	std::vector<sBoneAnimation> m_BoneAnimation;
};

// 각 뼈댜마다 정점들을 Bind Space에서 Bone Space로 변환하는 Offset 공간을 저장.
// 골격 계통 구조를 저장해야 하는 수단또한 필요.
// 또한 가지고 있는 Animation Clip을 저장하기 위한 구조체가 필요함.
class cSkinnedData
{
public:
	cSkinnedData();
	~cSkinnedData();

	UINT GetBoneCount() const
	{
		return m_BoneCount;
	}

	float GetCurClipStartTime(const std::string& clipName) const;
	float GetCurClipEndTime(const std::string& clipName) const;


	// 뼈대의 계층구조의 인덱스 값을 담는 Vector,
	// Bone의 Offset 변환 행렬을 담기 위한 Vector<XMFLOAT4X4>
	// Animation Clip을 Map 에 담기 위해 필요한 Set 함수도 필요.
	void Set(
		std::vector<int>& BoneHierarchy,
		std::vector<XMFLOAT4X4>& offsets,
		std::map<std::string, sAnimationClips>& aniClipTable
	);

	void GetFinalMatrix(const std::string& clipName, float t, std::vector<XMFLOAT4X4>& finalTranstorm);

private:
	std::vector<int> m_BoneHierarchyIdxArr;
	std::vector<XMFLOAT4X4> m_BoneOffsetMatArr;
	std::map<std::string, sAnimationClips> m_AnimationCilpTable;

	UINT m_BoneCount;



};

