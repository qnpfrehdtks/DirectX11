#pragma once

#include "DXUT.h"
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <vector>
#include <string>
#include <map>
#include "Vertex.h"
#include "LightMGR.h"

#include "MeshGeometry.h"
#include "SkinnedData.h"


using namespace Vertexs;


struct sCustomMaterial
{
	sMaterial Mat;
	bool AlphaClip;
	std::string EffectMapName;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
};

class cCustomFileLoader
{
public:
	cCustomFileLoader();

	void LoadFile(std::string filename,
		std::vector<sNormalVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::sMeshSet>& subsets,
		std::vector<sCustomMaterial>& mats);

	void LoadFile(std::string filename,
		std::vector<sSkinnedVertex>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::sMeshSet>& subsets,
		std::vector<sCustomMaterial>& mats,
	    cSkinnedData& skinInfo
	);


	~cCustomFileLoader();

	void ReadMaterial(std::ifstream& fin, UINT numMaterial, std::vector<sCustomMaterial>& mats);
	void ReadSubSet(std::ifstream& fin, UINT numSubSets, std::vector<MeshGeometry::sMeshSet>& mats);
	void ReadVertex(std::ifstream& fin, UINT numVertex, std::vector<sNormalVertex>& mats);
	
	void ReadTriangle(std::ifstream& fin, UINT numVertex, std::vector<USHORT>& mats);

	void ReadBoneOffsets(std::ifstream& fin, UINT numBone, std::vector<XMFLOAT4X4>& boneOffsetArr);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBone, std::vector<int>& boneHierarchy);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, sBoneAnimation& boneAnimation);

	void ReadSkinVertex(std::ifstream& fin, UINT numVertex, std::vector<sSkinnedVertex>& mats);
	void ReadAnimationClips(std::ifstream& fin, UINT numBone, UINT aniNum, std::map<std::string , sAnimationClips>& AniTable);
	
};

