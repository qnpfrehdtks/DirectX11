#include "DXUT.h"
#include "SkinnedAnimationData.h"
#include "MathHelper.h"

sAnimationClip::sAnimationClip() : m_CurTime(0)
{
}

void sAnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& finalMatrix)
{
	UINT size = m_BoneAnimations.size();

	for (int i = 0; i < size; i++)
	{
		if (m_BoneAnimations[i].m_KeyFrames.size() <= 0) continue;
		m_BoneAnimations[i].Interpolate(t, finalMatrix[i]);
	}
}

float sAnimationClip::GetEndAnimationTime() 
{
	float t = -MathHelper::Infinity;
	int BoneSize = m_BoneAnimations.size();
	for (auto i = 0; i < BoneSize; i++)
	{
		t = MathHelper::Max(t, m_BoneAnimations[i].GetEndTime());
	}

	return t;
}

float sAnimationClip::GetStartAnimationTime() 
{
	float t = MathHelper::Infinity;
	int BoneSize = m_BoneAnimations.size();
	for (auto i = 0; i < BoneSize; i++)
	{
		t = MathHelper::Min(t, m_BoneAnimations[i].GetStartTime());
	}

	return t;
}

void sBoneAni::Interpolate(float t, XMFLOAT4X4 & OutMat)
{
	if (t <= GetStartTime())
	{
		XMVECTOR S = XMLoadFloat3(&m_KeyFrames.front().GetScale());
		XMVECTOR Q = XMLoadFloat4(&m_KeyFrames.front().GetQuat());
		XMVECTOR P = XMLoadFloat3(&m_KeyFrames.front().GetPos());

		XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&OutMat, XMMatrixAffineTransformation(S, zero, Q, P));

	}
	else if (t >= GetEndTime())
	{
		XMVECTOR S = XMLoadFloat3(&m_KeyFrames.back().GetScale());
		XMVECTOR Q = XMLoadFloat4(&m_KeyFrames.back().GetQuat());
		XMVECTOR P = XMLoadFloat3(&m_KeyFrames.back().GetPos());

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&OutMat, XMMatrixAffineTransformation(S, zero, Q, P));

	}
	else
	{
		UINT size = m_KeyFrames.size() - 1;
		for (int i = 0; i < size; i++)
		{
			if (t >= m_KeyFrames[i].GetTime() && t <= m_KeyFrames[i + 1].GetTime())
			{
				float rate = (t - m_KeyFrames[i].GetTime()) / (m_KeyFrames[i + 1].GetTime() - m_KeyFrames[i].GetTime());


				XMVECTOR S0 = XMLoadFloat3(&m_KeyFrames[i].GetScale());
				XMVECTOR S1 = XMLoadFloat3(&m_KeyFrames[i + 1].GetScale());

				XMVECTOR Q0 = XMLoadFloat4(&m_KeyFrames[i].GetQuat());
				XMVECTOR Q1 = XMLoadFloat4(&m_KeyFrames[i + 1].GetQuat());

				XMVECTOR P0 = XMLoadFloat3(&m_KeyFrames[i].GetPos());
				XMVECTOR P1 = XMLoadFloat3(&m_KeyFrames[i + 1].GetPos());


				XMVECTOR S = XMVectorLerp(S0, S1, rate);
				XMVECTOR P = XMVectorLerp(P0, P1, rate);
				XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, rate);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&OutMat, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}




}

KeyFrame::KeyFrame()
{
}

KeyFrame::~KeyFrame()
{
}

float cSkinnedAnimation::GetCurClipStartTime(const std::string & clipName)
{
	auto clip = m_AnimationClipTable.find(clipName);
	//	if (clip == m_AnimationCilpTable.end()) return 0.0f;
	return clip->second.GetStartAnimationTime();
}

float cSkinnedAnimation::GetCurClipEndTime(const std::string & clipName)
{
	auto clip = m_AnimationClipTable.find(clipName);
	//	if (clip == m_AnimationCilpTable.end()) return 0.0f;
	return clip->second.GetEndAnimationTime();
}

void cSkinnedAnimation::GetFinalMatrix(const std::string & clipName, float t, std::vector<XMFLOAT4X4>& finalTranstorm)
{
	UINT boneSize =  m_BoneOffset.size();

	std::vector<XMFLOAT4X4> toParentTransforms(boneSize);

	// 애니메이션의 Bone에 있는 모든 값을 시간에 맞추어 보간 시켜 놓는다.
	// 보간 시킨 값은 그 해당 시간의 Bone의 좌표 변환 행렬 값
	auto clip = m_AnimationClipTable.find(clipName);
	clip->second.Interpolate(t, toParentTransforms);

	// toParent 행렬과 toRoot 변환 행렬 두가지를 준비한다.
	// 이유는 해당 Bone을 Root 좌표계로 옮기기 위해,
	// 그러기 위해서 toParent, toRoot(부모 좌표계의 Root 변환 행렬), Offset 변환 행렬 3가지를 준비함.
	std::vector<XMFLOAT4X4> toRootTransforms(boneSize);

	// 0번째 idx는 Root를 가르킴. 그 Root가 Root로 가는 변환 행렬은 부모 변환 행렬로 집어넣음.
	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 1; i < boneSize; i++)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		// 조상 Bone의 인덱스 값.
		UINT parentIdx = m_BoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIdx]);
		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (UINT i = 0; i < boneSize; i++)
	{
		XMMATRIX offset = XMLoadFloat4x4(&m_BoneOffset[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTranstorm[i], XMMatrixMultiply(offset, toRoot));
	}





}

