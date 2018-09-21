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

	// 애니메이션 처음 이라면
	if (dt <= Keyframes.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR T = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().Quaternion);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, T));

	}
	// 애니메이션의 실행이 다 끝나면.
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
				// Lerp를 위해 다음 키프레임를 위해 시간의 퍼센테이지를 구해준다.
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


// Animation Clip 중에서 가장 작은 Animation Start 시간을 가진 값을 가져옴.
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


// Animation Clip 중에서 가장 큰 Animation End 시간을 가진 값을 가져옴.
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

	// 애니메이션의 Bone에 있는 모든 값을 시간에 맞추어 보간 시켜 놓는다.
	// 보간 시킨 값은 그 해당 시간의 Bone의 좌표 변환 행렬 값
	auto clip = m_AnimationCilpTable.find(clipName);
	clip->second.Interpolate(t, toParentTransforms);

	// toParent 행렬과 toRoot 변환 행렬 두가지를 준비한다.
	 // 이유는 해당 Bone을 Root 좌표계로 옮기기 위해,
	// 그러기 위해서 toParent, toRoot(부모 좌표계의 Root 변환 행렬), Offset 변환 행렬 3가지를 준비함.
	std::vector<XMFLOAT4X4> toRootTransforms(boneSize);

	// 0번째 idx는 Root를 가르킴. 그 Root가 Root로 가는 변환 행렬은 부모 변환 행렬로 집어넣음.
	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 1; i <  boneSize; i++)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
		
		// 조상 Bone의 인덱스 값.
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
