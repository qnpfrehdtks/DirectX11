

#include "MeshGeometry.h"
#include "TextureResourceMGR.h"
#include "Vertex.h"
#include "DXUT.h"
#include <map>
#include "LightMGR.h"


struct KeyFrame
{
	KeyFrame();
	~KeyFrame();

	void Set(float Time, XMFLOAT3 pos, XMFLOAT4 rot, XMFLOAT3 scale)
	{
		m_Time = Time;
		m_Pos = pos;
		m_Quaternion = rot;
		m_Scale = scale;
	}

public:
	float GetTime() const {	return m_Time; }
	XMFLOAT3 GetPos() { return m_Pos; }
	XMFLOAT4 GetQuat() { return m_Quaternion; }
	XMFLOAT3 GetScale() { return m_Scale; }

private:
	float m_Time;
	XMFLOAT3 m_Pos;
	XMFLOAT4 m_Quaternion;
	XMFLOAT3 m_Scale;

};


struct sBoneAni
{
	
public:
	sBoneAni() {} ;
	~sBoneAni() {};

	std::string BoneName;
	int m_BoneIdx;
	std::vector<KeyFrame> m_KeyFrames;
	
	void Interpolate(float t, XMFLOAT4X4& OutMat);

	float GetStartTime() { return m_KeyFrames.front().GetTime(); }
	float GetEndTime() { return m_KeyFrames.back().GetTime(); }



};

struct sAnimationClip
{

	sAnimationClip();
	~sAnimationClip() {};

	std::vector<sBoneAni> m_BoneAnimations;
	float m_CurTime;

	void Interpolate(float t, std::vector<XMFLOAT4X4> &finalMatrix);

	// =======================================
	//                  GET
	// =======================================
	float GetEndAnimationTime();
	float GetStartAnimationTime();
};

class cSkinnedAnimation
{
private:
	UINT m_RootBoneIdx;
	std::vector<int> m_BoneHierarchy;
	std::vector<XMFLOAT4X4> m_BoneOffset;
	std::map<std::string, sAnimationClip> m_AnimationClipTable;
	//std::unordered_map<std::string, aiNodeAnim*> m_AnimationNodeTable;
public:
	cSkinnedAnimation() : m_RootBoneIdx(0) {}
	~cSkinnedAnimation() {}

	// =======================================
	//                  GET
	// =======================================
	float GetCurClipStartTime(const std::string & clipName);
	float GetCurClipEndTime(const std::string & clipName);
	void GetFinalMatrix(const std::string& clipName, float t, std::vector<XMFLOAT4X4>& finalTranstorm);

	UINT GetBoneSize() { return m_BoneOffset.size(); }
	std::map<std::string, sAnimationClip>& GetAniTable() { return m_AnimationClipTable; }
	// =======================================
	//                  SET
	// =======================================
	void SetAni( std::string aniStr ,sAnimationClip& ani) { m_AnimationClipTable[aniStr] = ani; }
	void SetBoneIdx(UINT idx) { m_RootBoneIdx = idx; }

	void SetBoneHierarchyIdx(std::vector<int>& boneHierarchy) { m_BoneHierarchy = boneHierarchy; }
	void SetBoneHierarchyIdx(UINT ParentIdx, UINT childIDx) {

		UINT size = m_BoneHierarchy.size();

		if (size <= ParentIdx || size <= childIDx)
			m_BoneHierarchy[childIDx] = ParentIdx;
	}

	void SetBoneOffset(std::vector<XMFLOAT4X4>& boneOffset) { m_BoneOffset = boneOffset; }
	void SetBoneOffset(XMFLOAT4X4 offset) {
		m_BoneOffset.push_back(offset);
	}
};

