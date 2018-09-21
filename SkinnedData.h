#pragma once
#include "DXUT.h"
#include <vector>
#include <map>


// ����. Frank D Luna Animation Part;
using namespace DirectX;


// �� Key �� ��ġ�� ���ʹϾ�, ũ�⸦ �����ϱ� ���� Struct
struct sKeyFrame
{

	sKeyFrame();
	~sKeyFrame();

	float TimePos;
	XMFLOAT3 Scale;
	XMFLOAT3 Translation;
	XMFLOAT4 Quaternion;
};

// �� Key Frame�� ���� �ִϸ��̼��� ������ ����ü.
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

// ���� ������ �����ϴ� ���뺰 �ִϸ��̼ǵ��� ������ 
// �ִϸ��̼� Clip �̶� ��Ī�Ѵ�. 
// �׷��� ������ ���� �ִϸ��̼� �迭�� �ΰ� �ٴ�.
struct sAnimationClips
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;

	void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransform);

	std::vector<sBoneAnimation> m_BoneAnimation;
};

// �� �������� �������� Bind Space���� Bone Space�� ��ȯ�ϴ� Offset ������ ����.
// ��� ���� ������ �����ؾ� �ϴ� ���ܶ��� �ʿ�.
// ���� ������ �ִ� Animation Clip�� �����ϱ� ���� ����ü�� �ʿ���.
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


	// ������ ���������� �ε��� ���� ��� Vector,
	// Bone�� Offset ��ȯ ����� ��� ���� Vector<XMFLOAT4X4>
	// Animation Clip�� Map �� ��� ���� �ʿ��� Set �Լ��� �ʿ�.
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

