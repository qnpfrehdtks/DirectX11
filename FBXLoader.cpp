#include "DXUT.h"
#include "FBXLoader.h"


FBXLoader::FBXLoader() : m_IdxCount(0)
{
	
}


FBXLoader::~FBXLoader()
{
}

void FBXLoader::InitFBXSDK()
{
	// FBX Sample Manaul ����.
	// FBX MGR ����.
	// FBX SDK �� �޸� ������, FBX SDK �� ��ü�� �ν��Ͻ�ȸ �� ������ ����, ���������� ����ϰ�, ��ü��
	// ������ ���� �ٽ� �����.

	// 1. SDK �����ڸ� ����� �����ϰ�, 
	// 2. �� ��ü�� ����Ͽ� �� ��ü�� �����.
	// 3. �� ��ü�� ����Ͽ� ��κ��� FBX SDK Ŭ������ ��ü�� ����.

	m_pFbxMgr = FbxManager::Create();

	if (!m_pFbxMgr)
	{
		exit(1);
	}

	// IOSetting object ����, �� object�� import export Object�� ����.
	// FbxIOSetting Ŭ������ ���� ��Ҹ� ���Ͽ��� �������ų� ���Ϸ� Export ���� ���θ� ������.
	// export�� ��Ҵ� Cam, Light, Mesh, Texture, Material Animation ����� ���� �Ӽ����� ����.
	// FBXIOSetting ����, Fbx �̱����� ����Ͽ� �����ǰ� ������.

	// IOSettings ��ü ���� �� ����.
	FbxIOSettings* ios = FbxIOSettings::Create(m_pFbxMgr, IOSROOT);
	m_pFbxMgr->SetIOSettings(ios);

	// ���డ���� ���丮���� �÷������� �ҷ��´�.
	FbxString Path = FbxGetApplicationDirectory();
	m_pFbxMgr->LoadPluginsDirectory(Path.Buffer());

	// FBX Scene �� ����,  Scene �� ���
	// Scene�� ����, �ؽ�ó, ĳ���� ���� �� �����ϰ� ����. 
	// �̷��� ��ҵ��� �ڽ� �����ؾ� �ϴ� ���� �����Ͽ� �����Ǿ� ��.
	// ���� ���� ����� �� ��� ��ҵ� ���� ����ȴ�. 
	// ���� ���� FBXNode�� Ʈ�� ������ �̷���� ����. Node�� ���� �������� �����ϴ� �����̳ʿ� ����. ���� ��� ī�޶� �޽���, �ִϸ��̼�
	// �׸��� �̷� Ư���� FbxNodeAttribute �� ������ �� �ִ�. 
	// FbxNode�� ���� FbxNodeAttribute�� ���� �� �� �ִ�.
	m_pFbxScene = FbxScene::Create(m_pFbxMgr, "myScene");

	if (!m_pFbxScene)
	{
		exit(1);
	}



	//g_pFbxScene->GetRootNode();
}

void FBXLoader::InitAssimp(
	std::string filename, 
	ID3D11Device* device, ID3D11DeviceContext* dc )
{
	Assimp::Importer importer;
	m_pAiScene = importer.ReadFile(
		filename, 
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (!m_pAiScene)
	{
		exit(1);
	}
	
	ProcessAssimp(m_pAiScene->mRootNode, m_pAiScene);


}

void FBXLoader::ProcessAssimp(
	aiNode * node, 
	const aiScene * scene)
{

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		
		ProcessMesh(mesh, scene, m_vertices[i], m_indices[i]);
	}
	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessAssimp(node->mChildren[i], scene);

	}

}

void FBXLoader::ProcessMesh(
	aiMesh * mesh, 
	const aiScene* scene, 
	std::vector<sBasicVertex>& vertices,
	std::vector<USHORT>& indices)
{
	
	//if (mesh->mMaterialIndex >= 0)
	//{
	//	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	//}

	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		sBasicVertex vertex;

		if (mesh->HasPositions())
		{
			vertex.Pos.x = mesh->mVertices[i].x;
			vertex.Pos.y = mesh->mVertices[i].y;
			vertex.Pos.z = mesh->mVertices[i].z;
		}
		if (mesh->HasNormals())
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}

		if (mesh->mTextureCoords[0])
		{
			vertex.Tex.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.Tex.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	UINT indicePerFaceCount = mesh->mFaces[0].mNumIndices;
	// NumFace ���� ����. �������� ����.
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		// �ϳ��� ���� ��������,
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	int a = 0;

}

void FBXLoader::LoadFile(const std::string filename, 
	std::vector<sBasicVertex>& vertices,
	std::vector<USHORT>& indices,
	std::vector<MeshGeometry::sMeshSet>& subsets, 
	std::vector<sCustomMaterial>& mats, 
	cSkinnedData & skinInfo)
{
	// Importer �ʱ�ȭ
	FbxImporter* importer = FbxImporter::Create(m_pFbxMgr, "");
	bool status = importer->Initialize(filename.c_str(), -1, m_pFbxMgr->GetIOSettings());

	if (!status)
	{
		exit(-1);
	}

	// FBX ������ Scene���� �������� ���ؼ� Impoter�� Scene�� Load �ؾ��Ѵ�.
	importer->Import(m_pFbxScene);

	// ���� Root Node ������.
	FbxNode* rootNode = m_pFbxScene->GetRootNode();

	// ��ǥ���� ��������, ������ ��ǥ���� �ٲ۴�.
	//FbxAxisSystem sceneAxisSystem = g_pFbxScene->GetGlobalSettings().GetAxisSystem();
	//FbxAxisSystem::MayaYUp.ConvertScene(g_pFbxScene);

	// �������� �ﰢ��ȭ �� �� �ִ� ��� ��带 �ﰢ��ȭ ��Ų��.
	FbxGeometryConverter geometryConverter(m_pFbxMgr);
	geometryConverter.Triangulate(m_pFbxScene, true);

	if (importer->GetAnimStackCount() >= 1)
		

	LoadFBX(rootNode, vertices, indices);

	importer->Destroy();

	int a = 0;

}

void FBXLoader::LoadFBX(FbxNode * node, std::vector<sBasicVertex>& vertexBuffer, std::vector<USHORT>& idxBuffer)
{
	FbxNodeAttribute* nodeAttri = node->GetNodeAttribute();

	if (nodeAttri)
	{
		if (nodeAttri->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh* mesh = node->GetMesh();

			UINT count = mesh->GetControlPointsCount();
			vertexBuffer.resize(count);

			for (int m = 0; m < count; m++)
			{
				XMFLOAT3 pos;
				vertexBuffer[m].Pos.x = static_cast<float>(mesh->GetControlPointAt(m).mData[0]);
				vertexBuffer[m].Pos.y = static_cast<float>(mesh->GetControlPointAt(m).mData[1]);
				vertexBuffer[m].Pos.z = static_cast<float>(mesh->GetControlPointAt(m).mData[2]);
			}

			UINT triCount = mesh->GetPolygonCount();
			idxBuffer.resize(triCount * 3);
			XMFLOAT3 normal;

			for (UINT j = 0; j < triCount; j++)
			{
				for (UINT k = 0; k < 3; k++)
				{
					int controlPintIndex = mesh->GetPolygonVertex(j, k);
					idxBuffer[j * 3 + k] = controlPintIndex;

					vertexBuffer[controlPintIndex].Normal = LoadNormal(mesh, controlPintIndex, m_IdxCount);
					vertexBuffer[controlPintIndex].Tex = LoadUV(mesh, controlPintIndex, mesh->GetTextureUVIndex(j,k));

					m_IdxCount++;
				}
			}
		}
	}
	const int childCount = node->GetChildCount();

	for (int i = 0; i < childCount; i++)
	{
		LoadFBX(node->GetChild(i), vertexBuffer, idxBuffer);
	}
}


XMFLOAT3 FBXLoader::LoadNormal(FbxMesh* mesh, int controlPointIndex, int vertexCount)
{
	FbxGeometryElementNormal* normalVertex = mesh->GetElementNormal(0);
	XMFLOAT3 result(0.0f, 0.0f, 0.0f);

	switch (normalVertex->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (normalVertex->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(normalVertex->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			result.y = static_cast<float>(normalVertex->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			result.z = static_cast<float>(normalVertex->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = normalVertex->GetIndexArray().GetAt(controlPointIndex);
			result.x = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[1]);
			result.z = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (normalVertex->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(normalVertex->GetDirectArray().GetAt(vertexCount).mData[0]);
			result.y = static_cast<float>(normalVertex->GetDirectArray().GetAt(vertexCount).mData[1]);
			result.z = static_cast<float>(normalVertex->GetDirectArray().GetAt(vertexCount).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = normalVertex->GetIndexArray().GetAt(vertexCount);
			result.x = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[1]);
			result.z = static_cast<float>(normalVertex->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			break;
		}
		break;
	}

	return result;
}

XMFLOAT2 FBXLoader::LoadUV(FbxMesh * mesh, int controlPointIndex, int vertexCount)
{
	FbxGeometryElementUV* textureUV = mesh->GetElementUV(0);
	XMFLOAT2 result(0.0f, 0.0f);

	switch (textureUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (textureUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(textureUV->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			result.y = static_cast<float>(textureUV->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = textureUV->GetIndexArray().GetAt(controlPointIndex);
			result.x = static_cast<float>(textureUV->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(textureUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (textureUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(textureUV->GetDirectArray().GetAt(vertexCount).mData[0]);
			result.y = static_cast<float>(textureUV->GetDirectArray().GetAt(vertexCount).mData[1]);
		
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = textureUV->GetIndexArray().GetAt(vertexCount);
			result.x = static_cast<float>(textureUV->GetDirectArray().GetAt(index).mData[0]);
			result.y = static_cast<float>(textureUV->GetDirectArray().GetAt(index).mData[1]);
			
		}
		break;

		default:
			break;
		}
		break;
	}

	return result;
}
