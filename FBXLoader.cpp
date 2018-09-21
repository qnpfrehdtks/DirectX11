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
	// FBX Sample Manaul 참고.
	// FBX MGR 생성.
	// FBX SDK 용 메모리 관리자, FBX SDK 는 객체를 인스턴스회 할 때마다 직접, 간접적으로 사용하고, 객체를
	// 삭제할 때도 다시 사용함.

	// 1. SDK 관리자를 만들어 시작하고, 
	// 2. 위 객체를 사용하여 씬 객체를 만든다.
	// 3. 씬 객체를 사용하여 대부분의 FBX SDK 클래스의 객체를 생성.

	m_pFbxMgr = FbxManager::Create();

	if (!m_pFbxMgr)
	{
		exit(1);
	}

	// IOSetting object 생성, 이 object는 import export Object를 생성.
	// FbxIOSetting 클래스는 씬의 요소를 파일에서 가져오거나 파일로 Export 할지 여부를 지정함.
	// export할 요소는 Cam, Light, Mesh, Texture, Material Animation 사용자 정의 속성등이 포함.
	// FBXIOSetting 또한, Fbx 싱글톤을 사용하여 생성되고 관리됨.

	// IOSettings 객체 생성 및 설정.
	FbxIOSettings* ios = FbxIOSettings::Create(m_pFbxMgr, IOSROOT);
	m_pFbxMgr->SetIOSettings(ios);

	// 실행가능한 디렉토리에서 플러그인을 불러온다.
	FbxString Path = FbxGetApplicationDirectory();
	m_pFbxMgr->LoadPluginsDirectory(Path.Buffer());

	// FBX Scene 을 생성,  Scene 은 대부
	// Scene은 재질, 텍스처, 캐릭터 포즈 를 포함하고 있음. 
	// 이러한 요소들은 자신 존재해야 하는 씬을 참조하여 생성되야 함.
	// 따라서 씬이 추출될 때 모든 요소도 같이 추출된다. 
	// 또한 씬은 FBXNode의 트리 구조롤 이루어져 있음. Node는 씬의 구성원을 포함하는 컨테이너와 같음. 예를 들어 카메라나 메쉬빛, 애니메이션
	// 그리고 이런 특성은 FbxNodeAttribute 로 가져올 수 있다. 
	// FbxNode당 여러 FbxNodeAttribute를 소유 할 수 있다.
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
	// NumFace 면의 갯수. 폴리곤의 갯수.
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		// 하나의 면을 가져오고,
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
	// Importer 초기화
	FbxImporter* importer = FbxImporter::Create(m_pFbxMgr, "");
	bool status = importer->Initialize(filename.c_str(), -1, m_pFbxMgr->GetIOSettings());

	if (!status)
	{
		exit(-1);
	}

	// FBX 내용을 Scene으로 가져오기 위해서 Impoter는 Scene을 Load 해야한다.
	importer->Import(m_pFbxScene);

	// 씬의 Root Node 가져옴.
	FbxNode* rootNode = m_pFbxScene->GetRootNode();

	// 좌표축을 가져오고, 씬내의 좌표축을 바꾼다.
	//FbxAxisSystem sceneAxisSystem = g_pFbxScene->GetGlobalSettings().GetAxisSystem();
	//FbxAxisSystem::MayaYUp.ConvertScene(g_pFbxScene);

	// 씬내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
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
