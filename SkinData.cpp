#include "DXUT.h"
#include "SkinData.h"
#include "SkinnedAnimationData.h"


void sSkinMesh::SetIndices(const USHORT * indices, UINT count)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(m_pDevice->CreateBuffer(&ibd, &iinitData, &m_IB));
}

void sSkinMesh::Draw(ID3D11DeviceContext * dc, UINT stride)
{
	UINT offset = 0;
	dc->IASetIndexBuffer(m_IB, DXGI_FORMAT_R16_UINT, offset);
	dc->IASetVertexBuffers(0, 1, &m_VB, &stride, &offset);

	dc->DrawIndexed(m_IndexCount, 0, 0);
}

void sSkinMesh::InsertTexture(std::string filename, aiTextureType type)
{
	sTEXTURE newTexture;

	newTexture.fileName = filename;
	newTexture.type = type;


	m_Textures.push_back(newTexture);
	
}


