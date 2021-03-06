#include "DXUT.h"
#include "AssimpFBXLoader.h"
#include "TextureResourceMGR.h"
#include "SkinData.h"
#include "SkinnedAnimationData.h"
#include "MathHelper.h"



AssimpFBXLoader::AssimpFBXLoader() : m_BoneCount(0)
{
	
}


AssimpFBXLoader::~AssimpFBXLoader()
{
}

void AssimpFBXLoader::LoadFBXFile(
	std::string filename, 
	std::string skinName,
	ID3D11Device * device, 
	ID3D11DeviceContext * dc,
	std::vector<sSkinMesh> &meshes, 
	cSkinnedAnimation* skinAni,
	std::vector<sTEXTURE>& texes,
	AnimationMGR* aniMGR
	)
{
	//// 중복일 시
	//if (m_SkinMeshTable.find(skinName) != m_SkinMeshTable.end())
	//{
	//	return;
	//}

	m_pDevice = device;

	
	m_pAiScene = m_Importer.ReadFile(filename,
		aiProcess_Triangulate | aiProcess_GenNormals |
		 aiProcess_CalcTangentSpace | 
		aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded  );

	if (!m_pAiScene)
	{
		exit(1);
	}

	

	meshes.reserve(m_pAiScene->mNumMeshes);

	for (UINT i = 0; i < m_pAiScene->mNumMeshes; i++)
	{
		aiMesh* paiMesh = m_pAiScene->mMeshes[i];
		sSkinMesh newmMesh;

		

		// Bone이 0개를 넘으면 SkinModel로 간주하고 0개 이하면 그냥 Model로 간주하고 Model 불러옴.
		LoadMesh(paiMesh, m_pAiScene->mRootNode, newmMesh, paiMesh->HasBones(), aniMGR);
     	
		meshes.push_back(newmMesh);
	}

	if (m_pAiScene->HasAnimations() && aniMGR)
	  aniMGR->Init(m_pAiScene);
 
	int Size = m_NodeInfoTable.size();
	Size = m_BoneInfoTable.size();
	Size = m_BoneMappingTable.size();
	Size = m_Tex.size();
}

void AssimpFBXLoader::CalcInterpolatedScaling(XMFLOAT3 & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out.x = pNodeAnim->mScalingKeys[0].mValue.x;
		Out.y = pNodeAnim->mScalingKeys[0].mValue.y;
		Out.z = pNodeAnim->mScalingKeys[0].mValue.z;
		return;
	}

	// Pos의 키
	UINT ScalingIndex = FindScalingAni(AnimationTime, pNodeAnim);
	UINT NextScalingIndex = (ScalingIndex + 1);

	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);

	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;

	assert(Factor >= 0.0f && Factor <= 1.0f);

	XMVECTOR Start = XMVectorSet(
		pNodeAnim->mScalingKeys[ScalingIndex].mValue.x,
		pNodeAnim->mScalingKeys[ScalingIndex].mValue.y,
		pNodeAnim->mScalingKeys[ScalingIndex].mValue.z, 0.0f);


	XMVECTOR End = XMVectorSet(
		pNodeAnim->mScalingKeys[NextScalingIndex].mValue.x,
		pNodeAnim->mScalingKeys[NextScalingIndex].mValue.y,
		pNodeAnim->mScalingKeys[NextScalingIndex].mValue.z, 0.0f);

	XMVECTOR Delta = XMVectorLerp(Start, End, Factor);
	XMStoreFloat3(&Out, Delta);
}

void AssimpFBXLoader::CalcInterpolatedRotation(XMFLOAT4 & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out.x = pNodeAnim->mRotationKeys[0].mValue.x;
		Out.y = pNodeAnim->mRotationKeys[0].mValue.y;
		Out.z = pNodeAnim->mRotationKeys[0].mValue.z;
		Out.w = pNodeAnim->mRotationKeys[0].mValue.w;
		return;
	}

	// Pos의 키
	UINT RotateIndex = FindRotationAni(AnimationTime, pNodeAnim);
	UINT NextRotateIndex = (RotateIndex + 1);

	assert(NextRotateIndex < pNodeAnim->mNumRotationKeys);

	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotateIndex].mTime - pNodeAnim->mRotationKeys[RotateIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotateIndex].mTime) / DeltaTime;

	assert(Factor >= 0.0f && Factor <= 1.0f);
	// ====================================================
	/*aiQuaternion out;

	aiQuaternion start = pNodeAnim->mRotationKeys[RotateIndex].mValue;
	aiQuaternion end = pNodeAnim->mRotationKeys[NextRotateIndex].mValue;
	aiQuaternion::Interpolate(out, start, end, Factor);
	out = out.Normalize();

	Out.x = out.x;
	Out.y = out.y;
	Out.z = out.z;
	Out.w = out.w;*/
	// ====================================================

	XMVECTOR Start = XMVectorSet(
		pNodeAnim->mRotationKeys[RotateIndex].mValue.x,
		pNodeAnim->mRotationKeys[RotateIndex].mValue.y,
		pNodeAnim->mRotationKeys[RotateIndex].mValue.z, 
		pNodeAnim->mRotationKeys[RotateIndex].mValue.w);


	XMVECTOR End = XMVectorSet(
		pNodeAnim->mRotationKeys[NextRotateIndex].mValue.x,
		pNodeAnim->mRotationKeys[NextRotateIndex].mValue.y,
		pNodeAnim->mRotationKeys[NextRotateIndex].mValue.z, 
		pNodeAnim->mRotationKeys[NextRotateIndex].mValue.w);

	XMVECTOR Delta = XMQuaternionSlerp(Start, End, Factor);
	Delta = XMQuaternionNormalize(Delta);
	XMStoreFloat4(&Out, Delta);
}

void AssimpFBXLoader::CalcInterpolatedPosition(XMFLOAT3 & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out.x = pNodeAnim->mPositionKeys[0].mValue.x;
		Out.y = pNodeAnim->mPositionKeys[0].mValue.y;
		Out.z = pNodeAnim->mPositionKeys[0].mValue.z;
		return;
	}

	// Pos의 키
	UINT PositionIndex = FindPositiongAni(AnimationTime, pNodeAnim);
	UINT NextPositionIndex = (PositionIndex + 1);

	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);

	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;

	assert(Factor >= 0.0f && Factor <= 1.0f);

	/*const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	aiVector3D out;
	out = Start + Factor * Delta;

	Out.x = out.x;
	Out.y = out.y;
	Out.z = out.z;*/
	XMVECTOR Start = XMVectorSet(
		pNodeAnim->mPositionKeys[PositionIndex].mValue.x,
		pNodeAnim->mPositionKeys[PositionIndex].mValue.y,
		pNodeAnim->mPositionKeys[PositionIndex].mValue.z, 0.0f);


	XMVECTOR End = XMVectorSet(
		pNodeAnim->mPositionKeys[NextPositionIndex].mValue.x,
		pNodeAnim->mPositionKeys[NextPositionIndex].mValue.y,
		pNodeAnim->mPositionKeys[NextPositionIndex].mValue.z, 0.0f);

	XMVECTOR Delta = XMVectorLerp(Start, End, Factor);
	XMStoreFloat3( &Out, Delta);
}

UINT AssimpFBXLoader::FindScalingAni(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	int scaleKeyNum = pNodeAnim->mNumScalingKeys;

	for (int i = 0; i < scaleKeyNum - 1; i++)
	{
		if (AnimationTime < pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}
}

UINT AssimpFBXLoader::FindRotationAni(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	int RotKeyNum = pNodeAnim->mNumRotationKeys;

	for (int i = 0; i < RotKeyNum - 1; i++)
	{
		if (AnimationTime < pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}
}

UINT AssimpFBXLoader::FindPositiongAni(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	int PosKeyNum = pNodeAnim->mNumPositionKeys;

	for (int i = 0; i < PosKeyNum - 1; i++)
	{
		if (AnimationTime < pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}
}

void AssimpFBXLoader::LoadBone(aiMesh * mesh, aiNode* node, std::vector<sSkinnedVertex>& verticeVec, UINT BaseVertex)
{

	std::vector<int> vec;
	//m_BoneTable.resize(mesh->mNumBones);
	// 해당 Mesh의 Bone의 값을 가져옴.
	for (int i = 0; i < mesh->mNumBones; i++)
	{
		std::string boneName = mesh->mBones[i]->mName.C_Str();

		int BoneIDX = 0;
		
		// 만약 Bone 입력이 처음이다. 그럼
		// Bone IDX를 mapping table에 새로 담자.
		if (m_BoneMappingTable.find(boneName) == m_BoneMappingTable.end())
		{
			BoneIDX = m_BoneCount;
			m_BoneCount++;

			sBone bone;
			m_BoneInfoTable.push_back(bone);
			aiMatrixToXMMatrix(mesh->mBones[i]->mOffsetMatrix, m_BoneInfoTable[BoneIDX].m_Offset);
		//	XMMATRIX temp = XMMatrixTranspose(XMLoadFloat4x4(&m_BoneInfoTable[BoneIDX].m_Offset));
		//	XMStoreFloat4x4(&m_BoneInfoTable[BoneIDX].m_Offset, temp);

			m_BoneMappingTable[boneName] = BoneIDX;

		}
		else
		{
			BoneIDX = m_BoneMappingTable[boneName];
		}

		// Bone에 저장되어 있는 Verteice 의 값 가져오기.

		for (int k = 0; k < mesh->mBones[i]->mNumWeights; k++)
		{
			UINT VertexID = mesh->mBones[i]->mWeights[k].mVertexId;

			// Vertex의 Pos, Normal, Tex, Tangent를 채우는 함수.
			LoadVertex(mesh, verticeVec, VertexID);

			////======================================================
			//// Vertex 의 Bone 정보를 채우는 부분.
			for (int j = 0; j < m_BoneMaxIdx; j++)
			{
				// 아직 덜 채운 부분을 채울것.
				if (verticeVec[VertexID].Weights[j] < 0.000001f)
				{
					if(j < 3)
					verticeVec[VertexID].Weights[j] = mesh->mBones[i]->mWeights[k].mWeight;

					verticeVec[VertexID].BoneIndices[j] = BoneIDX;
					break;
				}
			}
			//=====================================================
		}

	}
	
}


// ===================================================================
//   애니메이션의 정보를 담고 있는  Node들을 Unordered Map 에 저장한다.
// ===================================================================
void AssimpFBXLoader::LoadAni(aiAnimation* ani, sAnimationClip& aniClip)
{
	// Channel이 그 Bone Node
	for (int i = 0; i < ani->mNumChannels; i++)
	{
		aiNodeAnim* aniNode = ani->mChannels[i];
		m_NodeAniTable[aniNode->mNodeName.data] = aniNode;
	}

}

void AssimpFBXLoader::LoadTextures(sSkinMesh & omesh, aiMesh * mesh, aiTextureType type)
{
	std::string Dir = "Models/";
	USHORT MatrialIdx = mesh->mMaterialIndex;
	const aiMaterial* pMaterial = m_pAiScene->mMaterials[MatrialIdx];

	int a = 0;

	if ((a = pMaterial->GetTextureCount(type)) > 0)
	{
		std::string FullPath;

		aiString Path;
		if (pMaterial->GetTexture(type, 0, &Path) == AI_SUCCESS) {

			FullPath = Dir + Path.data;

			/*if (FullPath == "Models/spnza_bricks_a_diff.tga")
			{
				int a = 0;
			}*/

			omesh.InsertTexture(FullPath.c_str(), type);
		}
	}
}


const aiNodeAnim* AssimpFBXLoader::FindAniNode(const aiAnimation * pAnimation, const std::string NodeName)
{
	UINT size = pAnimation->mNumChannels;
	for (int i = 0; i < size; i++)
	{
	    aiNodeAnim* anim = pAnimation->mChannels[i];

		if (anim->mNodeName.data == NodeName)
		{
			return anim;
		}
	}
	return nullptr;
}

void AssimpFBXLoader::LoadNode(
	aiNode* node,
	const aiScene* scene)
{
	m_NodeInfoTable.push_back(node);

	int NumChild = node->mNumChildren;
	for (int i = 0; i < NumChild; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}




//// 마지막으로 남은 부분, 즉, 모든 Bone의 부모 주소값을 담기 위해 존재하는 함수.
//// SkinAnimation 클래스가 해당 bone의 하이라키도 채워서 어떤 bone이 누구의 부모인지 알 수 있게 함.
//void AssimpFBXLoader::FinalLoadBone(
//	aiNode* node,
//	std::vector<int>& BoneHierarchy, 
//	std::vector<XMFLOAT4X4>& BoneOffsets)
//{
//	UINT size = m_BoneTable.size();
//	BoneHierarchy.resize(size);
//	BoneOffsets.resize(size);
//
//	for ( auto iter = m_BoneTable.begin(); 
//		iter != m_BoneTable.end(); 
//		iter++)
//	{
//		std::string boneName = (*iter).second.
//		const aiNode* aiNodeiter = m_aiNodeTable[  ]
//
//		cBone* bone = &(*iter).second;
//
//		BoneOffsets[bone->m_BoneId] = bone->m_OffsetMat;
//		aiNode* ppNode = (*aiNodeiter).second->mParent;
//
//		if (ppNode != nullptr)
//		{
//			cBone* ppBone = &m_BoneTable[ppNode->mName.C_Str()];
//
//			if (ppBone->m_BoneId >= 0)
//			{
//				bone->m_ParentBone = ppBone;
//				BoneHierarchy[bone->m_BoneId] = bone->m_ParentBone->m_BoneId;
//			}
//			else 
//			{
//
//				BoneHierarchy[bone->m_BoneId] = -1;
//				bone->m_ParentBone = nullptr;
//			}
//		}
//	}
//}

void AssimpFBXLoader::LoadMesh(
	aiMesh * mesh,
	aiNode* node, sSkinMesh& omesh, BOOL isSkinnedModel, AnimationMGR* aniMGR)
{
	std::vector<sSkinnedVertex> verticeVec;
	std::vector<USHORT> indiceVec;

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < 3; j++) {
			indiceVec.push_back((USHORT)face.mIndices[j]);
		}
	}

	// =========================================
	//         Texture Loading
	// =========================================
	LoadTextures(omesh, mesh, aiTextureType_DIFFUSE);
	LoadTextures(omesh, mesh, aiTextureType_SPECULAR);
	LoadTextures(omesh, mesh, aiTextureType_NORMALS);
	LoadTextures(omesh, mesh, aiTextureType_OPACITY);
	// ==========================================================================
	//             Vertex 정보 불러와서 Index, Vertex Buffer 설정하기.
	// ==========================================================================
	verticeVec.resize(mesh->mNumVertices);

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		LoadVertex(mesh, verticeVec, i);
	}
	// Animation이 있다면, Bone Load
	if (isSkinnedModel && aniMGR != nullptr)
		aniMGR->LoadBone(mesh, verticeVec);
	// Mesh의 정보를 가지고, Vertex, Index Buffer 생성 마무리.
	omesh.Init(m_pDevice, verticeVec, indiceVec, 0);
	omesh.SetVertices(&verticeVec[0], verticeVec.size());
	omesh.SetIndices(&indiceVec[0], indiceVec.size());
}

void AssimpFBXLoader::LoadVertex(aiMesh * mesh, std::vector<sSkinnedVertex>& verticeVec, UINT vertexID)
{
	verticeVec[vertexID].Pos.x = mesh->mVertices[vertexID].x;
	verticeVec[vertexID].Pos.y = mesh->mVertices[vertexID].y;
	verticeVec[vertexID].Pos.z = mesh->mVertices[vertexID].z;

	verticeVec[vertexID].Normal.x = mesh->mNormals[vertexID].x;
	verticeVec[vertexID].Normal.y = mesh->mNormals[vertexID].y;
	verticeVec[vertexID].Normal.z = mesh->mNormals[vertexID].z;


	verticeVec[vertexID].Tex.x = mesh->mTextureCoords[0][vertexID].x;
	verticeVec[vertexID].Tex.y = mesh->mTextureCoords[0][vertexID].y;


	verticeVec[vertexID].TangentU.x = mesh->mTangents[vertexID].x;
	verticeVec[vertexID].TangentU.y = mesh->mTangents[vertexID].y;
	verticeVec[vertexID].TangentU.z = mesh->mTangents[vertexID].z;


}

void AssimpFBXLoader::ReadNodeHeirarchy(
	float AnimationTime, 
	const aiNode * pNode, 
	const XMFLOAT4X4& ParentTransform
	)
{

	std::string NodeName = pNode->mName.data;
	 aiAnimation* anim = m_pAiScene->mAnimations[0];

	///* XMFLOAT4X4 transform;

	 XMFLOAT4X4 transform;
	// XMStoreFloat4x4(&transform, XMMatrixIdentity());
	 aiMatrixToXMMatrix(pNode->mTransformation, transform);

	// XMMATRIX trmat = XMMatrixTranspose(XMLoadFloat4x4(&transform));
//	 XMStoreFloat4x4(&transform, trmat);

	 //aiNodeAnim에는 Animation과 관련된 정보가 삽입되어 있음.
	 auto pNodeAnim = m_NodeAniTable[NodeName];

	if (pNodeAnim)
	{
			//aiMatrixToXMMatrix(pNodeAnim->, transform);

			XMFLOAT3 Pos;
			CalcInterpolatedPosition(Pos, AnimationTime, pNodeAnim);

			XMFLOAT3 Scale;
			CalcInterpolatedScaling(Scale, AnimationTime, pNodeAnim);

			XMFLOAT4 Rot;
			CalcInterpolatedRotation(Rot, AnimationTime, pNodeAnim);

			aiVector3D  S, T;
			aiQuaternion R;
			S.x = Scale.x;
			S.y = Scale.y;
			S.z = Scale.z;

			R.x = Rot.x;
			R.y = Rot.y;
			R.z = Rot.z;
			R.w = Rot.w;

			T.x = Pos.x;
			T.y = Pos.y;
			T.z = Pos.z;

			aiMatrix4x4 scale, trans, rot;
			aiMatrix4x4::Scaling(S, scale);
			aiMatrix4x4::Translation(T, trans);
			rot = aiMatrix4x4(R.GetMatrix());

			aiMatrix4x4 NodeTransformation = trans * rot   * scale;

			aiMatrixToXMMatrix(NodeTransformation, transform);
			int a = 0;
	}

	XMMATRIX pMat = XMLoadFloat4x4(&ParentTransform);
	XMMATRIX Tr = XMLoadFloat4x4(&transform);

	XMMATRIX GlobalTransformation = XMMatrixMultiply(pMat , Tr);

	if (m_BoneMappingTable.find(NodeName) != m_BoneMappingTable.end())
	{
		int BoneIdx = m_BoneMappingTable[NodeName];
	//	XMStoreFloat4x4(&m_BoneInfoTable[idx].m_FinalMat, GlobalTransformation);
		XMMATRIX offset = XMLoadFloat4x4(&m_BoneInfoTable[BoneIdx].m_Offset);
		XMStoreFloat4x4(&m_BoneInfoTable[BoneIdx].m_FinalMat, XMLoadFloat4x4(&m_Inverse) *  GlobalTransformation  * offset);

	}

	XMFLOAT4X4 gTr;
	XMStoreFloat4x4(&gTr, GlobalTransformation);


	for (UINT i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], gTr);
	}


}

void AssimpFBXLoader::BoneTransform(float Time, std::vector<XMFLOAT4X4>& FinalTransform/*, sAnimationClip& aniClip*/)
{
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());

	float TicksPerSecond = (float)(m_pAiScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pAiScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = Time * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)m_pAiScene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, m_pAiScene->mRootNode, identity);

	FinalTransform.resize(m_BoneCount);

	for (int i = 0; i < m_BoneCount; i++)
	{
		FinalTransform[i] = m_BoneInfoTable[i].m_FinalMat;
		//XMStoreFloat4x4(&FinalTransform[i], XMMatrixIdentity());
	}


}



void AssimpFBXLoader::aiMatrixToXMMatrix(const aiMatrix4x4 & source, XMFLOAT4X4 & destMAt)
{
	destMAt._11 = source.a1;
	destMAt._12 = source.a2;
	destMAt._13 = source.a3;
	destMAt._14 = source.a4;

	destMAt._21 = source.b1;
	destMAt._22 = source.b2;
	destMAt._23 = source.b3;
	destMAt._24 = source.b4;

	destMAt._31 = source.c1;
	destMAt._32 = source.c2;
	destMAt._33 = source.c3;
	destMAt._34 = source.c4;

	destMAt._41 = source.d1;
	destMAt._42 = source.d2;
	destMAt._43 = source.d3;
	destMAt._44 = source.d4;


}

void AssimpFBXLoader::RightHandtoLeft(XMFLOAT4X4 & source)
{
	float t1 = source._14;
	float t2 = source._24;
	float t3 = source._34;
	float t4 = source._44;

	source._14 = source._41;
	source._24 = source._42;
	source._34 = source._43;
	source._44 = source._44;

	source._41 = t1;
	source._42 = t2;
	source._43 = t3;
	source._44 = t4;

	
	
}
