#include "DXUT.h"
#include "CustomFileLoader.h"



cCustomFileLoader::cCustomFileLoader()
{
}




void cCustomFileLoader::LoadFile(
	std::string filename, 
	std::vector<sSkinnedVertex>& vertices, 
	std::vector<USHORT>& indices, 
	std::vector<MeshGeometry::sMeshSet>& subsets, 
	std::vector<sCustomMaterial>& mats, 
	cSkinnedData & skinInfo)
{
	std::ifstream fin(filename);


	UINT MaterialNum = 0;
	UINT VertexNum = 0;
	UINT TriangleNum = 0;
	UINT BoneNum = 0;
	UINT AnimationClipNum = 0;

	std::vector<XMFLOAT4X4> BoneOffsetMats;
	std::vector<int> BoneHierarchyIDXs;
	std::map<std::string, sAnimationClips> AniTable;

	std::string ignore;
	

	if (fin)
	{
		fin >> ignore;
		fin >> ignore >> MaterialNum;
		fin >> ignore >> VertexNum;
		fin >> ignore >> TriangleNum;
		fin >> ignore >> BoneNum;
		fin >> ignore >> AnimationClipNum;

		ReadMaterial(fin, MaterialNum, mats);
		ReadSubSet(fin, MaterialNum, subsets);
		ReadSkinVertex(fin, VertexNum, vertices);
		ReadTriangle(fin, TriangleNum, indices);
		ReadBoneOffsets(fin, BoneNum, BoneOffsetMats);
		ReadBoneHierarchy(fin, BoneNum, BoneHierarchyIDXs);
		ReadAnimationClips(fin, BoneNum, AnimationClipNum, AniTable);
	}

	skinInfo.Set(BoneHierarchyIDXs, BoneOffsetMats, AniTable);
}



void cCustomFileLoader::LoadFile(
	std::string filename,
	std::vector<sNormalVertex>& vertices,
	std::vector<USHORT>& indices,
	std::vector<MeshGeometry::sMeshSet>& subsets,
	std::vector<sCustomMaterial>& mats)
{
	std::ifstream fin(filename);


	UINT MaterialNum = 0;
	UINT VertexNum = 0;
	UINT TriangleNum = 0;
	UINT BoneNum = 0;
	UINT AnimationClipNum = 0;

	std::string ignore;


	if (fin)
	{
		fin >> ignore;
		fin >> ignore >> MaterialNum;
		fin >> ignore >> VertexNum;
		fin >> ignore >> TriangleNum;
		fin >> ignore >> BoneNum;
		fin >> ignore >> AnimationClipNum;

		ReadMaterial(fin, MaterialNum, mats);
		ReadSubSet(fin, MaterialNum, subsets);
		ReadVertex(fin, VertexNum, vertices);
		ReadTriangle(fin, TriangleNum, indices);
	}
}


cCustomFileLoader::~cCustomFileLoader()
{
}

void cCustomFileLoader::ReadMaterial(std::ifstream & fin, UINT numMaterial, std::vector<sCustomMaterial>& mats)
{
	std::string ignore;

	mats.resize(numMaterial);

	std::string NormalMapName;
	std::string DiffuseMapName;


	fin >> ignore;

	for (int i = 0; i < numMaterial; i++)
	{

		fin >> ignore >> mats[i].Mat.Ambient.x >> mats[i].Mat.Ambient.y >> mats[i].Mat.Ambient.z;
		fin >> ignore >> mats[i].Mat.Diffuse.x >> mats[i].Mat.Diffuse.y >> mats[i].Mat.Diffuse.z;
		fin >> ignore >> mats[i].Mat.Specular.x >> mats[i].Mat.Specular.y >> mats[i].Mat.Specular.z;
		fin >> ignore >> mats[i].Mat.Specular.w ;
		fin >> ignore >> mats[i].Mat.Reflect.x >> mats[i].Mat.Reflect.y >> mats[i].Mat.Reflect.z;
		fin >> ignore >> mats[i].AlphaClip;
		fin >> ignore >> mats[i].EffectMapName;
		fin >> ignore >> DiffuseMapName;
		fin >> ignore >> NormalMapName;

		mats[i].DiffuseMapName.resize(DiffuseMapName.size(), ' ');
		mats[i].NormalMapName.resize(NormalMapName.size(), ' ');
		std::copy(DiffuseMapName.begin(), DiffuseMapName.end(), mats[i].DiffuseMapName.begin());
		std::copy(NormalMapName.begin(), NormalMapName.end(), mats[i].NormalMapName.begin());
	}
	int i = 0;

}

void cCustomFileLoader::ReadSubSet(std::ifstream & fin, UINT numSubSets, std::vector<MeshGeometry::sMeshSet>& mats)
{
	std::string ignore;

	mats.resize(numSubSets);


	fin >> ignore;

	for (int i = 0; i < numSubSets; i++)
	{
		fin >> ignore >> mats[i].ID;
		fin >> ignore >> mats[i].VertexStart;
		fin >> ignore >> mats[i].VertexCount;
		fin >> ignore >> mats[i].FaceStart;
		fin >> ignore >> mats[i].FaceCount;

	}

}

void cCustomFileLoader::ReadVertex(std::ifstream & fin, UINT numVertex, std::vector<sNormalVertex>& mats)
{
	std::string ignore;

	mats.resize(numVertex);

	fin >> ignore;

	for (int i = 0; i < numVertex; i++)
	{

		fin >> ignore >> mats[i].Pos.x >> mats[i].Pos.y >> mats[i].Pos.z;
		fin >> ignore >> mats[i].TangentU.x >> mats[i].TangentU.y >> mats[i].TangentU.z >> ignore;
		fin >> ignore >> mats[i].Normal.x >> mats[i].Normal.y >> mats[i].Normal.z;
		fin >> ignore >> mats[i].Tex.x >> mats[i].Tex.y;
		fin >> ignore >> mats[i].Tex.x >> mats[i].Tex.y;
		fin >> ignore >> mats[i].Tex.x >> mats[i].Tex.y;
	}
}


void cCustomFileLoader::ReadSkinVertex(std::ifstream & fin, UINT numVertex, std::vector<sSkinnedVertex>& mats)
{
	std::string ignore;

	mats.resize(numVertex);

	fin >> ignore;

	int BoneIndices[4];

	for (int i = 0; i < numVertex; i++)
	{

		fin >> ignore >> mats[i].Pos.x >> mats[i].Pos.y >> mats[i].Pos.z;
		fin >> ignore >> mats[i].TangentU.x >> mats[i].TangentU.y >> mats[i].TangentU.z >> ignore;
		fin >> ignore >> mats[i].Normal.x >> mats[i].Normal.y >> mats[i].Normal.z;
		fin >> ignore >> mats[i].Tex.x >> mats[i].Tex.y;
		fin >> ignore >> mats[i].Weights[0] >> mats[i].Weights[1] >> mats[i].Weights[2] >> ignore;
		fin >> ignore >> BoneIndices[0] >> BoneIndices[1] >> BoneIndices[2] >> BoneIndices[3];

		mats[i].BoneIndices[0] = (BYTE)BoneIndices[0];
		mats[i].BoneIndices[1] = (BYTE)BoneIndices[1];
		mats[i].BoneIndices[2] = (BYTE)BoneIndices[2];
		mats[i].BoneIndices[3] = (BYTE)BoneIndices[3];


	}
}

void cCustomFileLoader::ReadTriangle(std::ifstream & fin, UINT numVertex, std::vector<USHORT>& mats)
{
	std::string ignore;

	mats.resize(numVertex * 3);

	fin >> ignore;

	for (int i = 0; i < numVertex; i++)
	{
		fin >> mats[i * 3 + 0] >> mats[i * 3 + 1] >> mats[i * 3 + 2];
		
	}
}

void cCustomFileLoader::ReadBoneOffsets(std::ifstream & fin, UINT numBone, std::vector<XMFLOAT4X4>& boneOffsetArr)
{
	std::string ignore;
	boneOffsetArr.resize(numBone);
	fin >> ignore;
	for (int i = 0; i < numBone; i++)
	{
		if (i == 57)
		{
			int a = 0;
		}

		fin >> ignore >> boneOffsetArr[i](0, 0) >> boneOffsetArr[i](0, 1) >> boneOffsetArr[i](0, 2) >> boneOffsetArr[i](0, 3)
		>> boneOffsetArr[i](1, 0) >> boneOffsetArr[i](1, 1) >> boneOffsetArr[i](1, 2) >> boneOffsetArr[i](1, 3)
		 >> boneOffsetArr[i](2, 0) >> boneOffsetArr[i](2, 1) >> boneOffsetArr[i](2, 2) >> boneOffsetArr[i](2, 3)
		>> boneOffsetArr[i](3, 0) >> boneOffsetArr[i](3, 1) >> boneOffsetArr[i](3, 2) >> boneOffsetArr[i](3, 3);
	}


}

void cCustomFileLoader::ReadBoneHierarchy(std::ifstream & fin, UINT numBone, std::vector<int>& boneHierarchy)
{
	std::string ignore;
	boneHierarchy.resize(numBone);

	fin >> ignore;
	for (int i = 0; i < numBone; i++)
	{
		fin >> ignore >> boneHierarchy[i];
	}
	int a = 0;
}

void cCustomFileLoader::ReadBoneKeyframes(std::ifstream & fin, UINT numBones, sBoneAnimation & boneAnimation)
{
	std::string ignore;
	UINT KeyFrameCount;
	fin >> ignore >> ignore >> KeyFrameCount;
	boneAnimation.Keyframes.resize(KeyFrameCount);

	fin >> ignore;

	for (int i = 0; i < KeyFrameCount; i++)
	{
		float Time = 0.0f;
		XMFLOAT3 pos(0.0f, 0.0f, 0.0f);
		XMFLOAT3 scale(1.0f, 1.0f, 1.0f);
		XMFLOAT4 quat(0.0f, 0.0f, 0.0f, 1.0f);


		fin >> ignore >> Time;
		fin >> ignore >> pos.x >> pos.y >> pos.z;
		fin >> ignore >> scale.x >> scale.y >> scale.z;
		fin >> ignore >> quat.x >> quat.y >> quat.z >> quat.w;

		boneAnimation.Keyframes[i].TimePos = Time;
		boneAnimation.Keyframes[i].Translation = pos;
		boneAnimation.Keyframes[i].Scale = scale;
		boneAnimation.Keyframes[i].Quaternion = quat;
	}

	fin >> ignore;

}

void cCustomFileLoader::ReadAnimationClips(
	std::ifstream & fin,
	UINT numBone,
	UINT aniNum, 
	std::map<std::string, sAnimationClips>& AniTable)
{
	std::string ignore;
	fin >> ignore;
	int a = 0;
	for (int i = 0; i < aniNum; i++)
	{
		std::string AniClipName;
		UINT FrameCount;

		sAnimationClips newClip;

		fin >> ignore >> AniClipName;
		fin >> ignore;

		newClip.m_BoneAnimation.resize(numBone);

		for (UINT boneIdx = 0; boneIdx < numBone; boneIdx++)
		{
			ReadBoneKeyframes(fin, boneIdx, newClip.m_BoneAnimation[boneIdx]);
		}
		fin >> ignore;

		AniTable.insert( std::make_pair(AniClipName,newClip));
	}

	


}
