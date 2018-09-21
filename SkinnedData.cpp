#include "DXUT.h"
#include "SkinnedData.h"
#include "MathHelper.h"


sBoneAnimation::sBoneAnimation() 
{
}

sBoneAnimation::~sBoneAnimation()
{
}

void sBoneAnimation::Interpolate(float dt, XMFLOAT4X4 & M)
{
	//curTime += dt;
	//if (curTime >= GetEndTime())
	//{
		// Loop animation back to beginning.
		//curTime = 0.0f;
	//}

	// �ִϸ��̼� ó�� �̶��
	if (dt <= Keyframes.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR T = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().Quaternion);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, T));

	}
	// �ִϸ��̼��� ������ �� ������.
	else if (dt >= Keyframes.back().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR T = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().Quaternion);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, T));
	}
	else
	{
		int Size = Keyframes.size() - 1;

		for (int i = 0; i < Size; i++)
		{
			if (dt >= Keyframes[i].TimePos && dt <= Keyframes[i + 1].TimePos)
			{
				// Lerp�� ���� ���� Ű�����Ӹ� ���� �ð��� �ۼ��������� �����ش�.
				float LerpTime = (dt - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

				XMVECTOR t0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR t1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].Quaternion);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].Quaternion);

				XMVECTOR S = XMVectorLerp(s0, s1, LerpTime);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, LerpTime);
				XMVECTOR T = XMVectorLerp(t0, t1, LerpTime);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, T));

				break;
			}
		}
	}


}

sKeyFrame::sKeyFrame() :
	TimePos(0.0f), Scale(1.0f, 1.0f, 1.0f),
	Translation(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f)
{
}


sKeyFrame::~sKeyFrame()
{
}


// Animation Clip �߿��� ���� ���� Animation Start �ð��� ���� ���� ������.
float sAnimationClips::GetClipStartTime() const
{
	float t = MathHelper::Infinity;
	int BoneSize = m_BoneAnimation.size();
	for (auto i = 0; i < BoneSize; i++)
	{
		t = MathHelper::Min(t, m_BoneAnimation[i].GetStartTime());
	}

	return t;
}


// Animation Clip �߿��� ���� ū Animation End �ð��� ���� ���� ������.
float sAnimationClips::GetClipEndTime() const
{
	float t = -MathHelper::Infinity;
	int BoneSize = m_BoneAnimation.size();
	for (auto i = 0; i < BoneSize; i++)
	{
		t = MathHelper::Max(t, m_BoneAnimation[i].GetEndTime());
	}

	return t;
}

void sAnimationClips::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransform)
{
	int BoneSize = m_BoneAnimation.size();

	for (int i = 0; i < BoneSize; i++)
	{
		m_BoneAnimation[i].Interpolate(t, boneTransform[i]);
	}



}

cSkinnedData::cSkinnedData()
{
}

cSkinnedData::~cSkinnedData()
{
}

float cSkinnedData::GetCurClipStartTime(const std::string & clipName) const
{
	auto clip = m_AnimationCilpTable.find(clipName);
//	if (clip == m_AnimationCilpTable.end()) return 0.0f;
	return clip->second.GetClipStartTime();
}

float cSkinnedData::GetCurClipEndTime(const std::string & clipName) const
{
	auto clip = m_AnimationCilpTable.find(clipName);
	//if (clip == m_AnimationCilpTable.end()) return 0.0f;
	return clip->second.GetClipEndTime();
}

void cSkinnedData::Set(
	std::vector<int>& BoneHierarchy, 
	std::vector<XMFLOAT4X4>& offsets, 
	std::map<std::string, sAnimationClips>& aniClipTable )
{
	m_BoneCount = offsets.size();
	m_BoneHierarchyIdxArr = BoneHierarchy;
	m_BoneOffsetMatArr = offsets;
	m_AnimationCilpTable = aniClipTable;
}

//  toParent * toRoot(Parent)


void cSkinnedData::GetFinalMatrix(const std::string & clipName, float t, std::vector<XMFLOAT4X4>& finalTranstorm)
{
	UINT boneSize = m_BoneOffsetMatArr.size();

	std::vector<XMFLOAT4X4> toParentTransforms(boneSize);

	// �ִϸ��̼��� Bone�� �ִ� ��� ���� �ð��� ���߾� ���� ���� ���´�.
	// ���� ��Ų ���� �� �ش� �ð��� Bone�� ��ǥ ��ȯ ��� ��
	auto clip = m_AnimationCilpTable.find(clipName);
	clip->second.Interpolate(t, toParentTransforms);

	// toParent ��İ� toRoot ��ȯ ��� �ΰ����� �غ��Ѵ�.
	 // ������ �ش� Bone�� Root ��ǥ��� �ű�� ����,
	// �׷��� ���ؼ� toParent, toRoot(�θ� ��ǥ���� Root ��ȯ ���), Offset ��ȯ ��� 3������ �غ���.
	std::vector<XMFLOAT4X4> toRootTransforms(boneSize);

	// 0��° idx�� Root�� ����Ŵ. �� Root�� Root�� ���� ��ȯ ����� �θ� ��ȯ ��ķ� �������.
	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 1; i <  boneSize; i++)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
		
		// ���� Bone�� �ε��� ��.
		UINT parentIdx = m_BoneHierarchyIdxArr[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIdx]);
		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (UINT i = 0; i < boneSize; i++)
	{
		XMMATRIX offset = XMLoadFloat4x4(& m_BoneOffsetMatArr[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTranstorm[i], XMMatrixMultiply(offset, toRoot));
	}


	int a = 0;

}
