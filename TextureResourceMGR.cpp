#include "DXUT.h"
#include "TextureResourceMGR.h"
#include "DDSTextureLoader.h"
#include "resource.h"
#include "WICTextureLoader.h"
#include <DirectXTex.h>

using namespace DirectX;

TextureResourceMGR::TextureResourceMGR()
{
	
}


TextureResourceMGR::~TextureResourceMGR()
{
}

ID3D11ShaderResourceView* TextureResourceMGR::CreateSRV(ID3D11Device* pDevice, std::wstring filename)
{
	
	ID3D11ShaderResourceView* srv = m_ResourceMap[filename];

	if (srv != nullptr) return srv;

	HR(CreateDDSTextureFromFile(pDevice, filename.c_str(), nullptr, &srv));
	InsertTextureView(srv, filename);

	return srv;

}

ID3D11ShaderResourceView * TextureResourceMGR::CreateSRVFromTGA(ID3D11Device * pDevice, std::wstring filename)
{
	auto file = m_ResourceMap.find(filename);


	if (file != m_ResourceMap.end()) return (*file).second;


	ScratchImage image;
	HR(LoadFromTGAFile(filename.c_str(), nullptr, image));


	ID3D11ShaderResourceView* srv = nullptr;

	HR(CreateShaderResourceView(pDevice,
		image.GetImages(), image.GetImageCount(),
		image.GetMetadata(), &srv));

	InsertTextureView(srv, filename);



	return srv;
}

ID3D11ShaderResourceView * TextureResourceMGR::CreateSRVFromWIC(ID3D11Device * pDevice, std::wstring filename)
{
	auto file = m_ResourceMap.find(filename);


	if (file != m_ResourceMap.end()) return (*file).second;


	ScratchImage image;
	HR(LoadFromWICFile(filename.c_str(), WIC_FLAGS_NONE, nullptr, image));
	

	ID3D11ShaderResourceView* srv = nullptr;

	HR(CreateShaderResourceView(pDevice,
		image.GetImages(), image.GetImageCount(),
		image.GetMetadata(), &srv));

	InsertTextureView(srv, filename);



	return srv;
}

ID3D11ShaderResourceView * TextureResourceMGR::CreateSRVArrFromDDS(ID3D11Device * pDevice, ID3D11DeviceContext * pDC, std::vector<std::wstring> filenames)
{

	 UINT size = filenames.size();
	 std::vector<ID3D11Texture2D*> srcTex(size);

	 for (int i = 0; i < 25; i++)
	 {
		 HR(CreateWICTextureFromFileEx(pDevice, nullptr, filenames[i].c_str(), 0,
			 D3D11_USAGE_STAGING, 0 ,  D3D11_CPU_ACCESS_READ, 0, 0,
			 (ID3D11Resource**)&srcTex[i], 0));
	 };

	 D3D11_TEXTURE2D_DESC texElementDesc;
	 srcTex[0]->GetDesc(&texElementDesc);


	 D3D11_TEXTURE2D_DESC texArrayDesc;
	 texArrayDesc.Width = texElementDesc.Width;
	 texArrayDesc.Height = texElementDesc.Height;
	 texArrayDesc.MipLevels = texElementDesc.MipLevels;
	 texArrayDesc.ArraySize = size;
	 texArrayDesc.Format = texElementDesc.Format;
	 texArrayDesc.SampleDesc.Count = 1;
	 texArrayDesc.SampleDesc.Quality = 0;
	 texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	 texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	 texArrayDesc.CPUAccessFlags = 0;
	 texArrayDesc.MiscFlags = 0;

	 ID3D11Texture2D* texArray = 0;
	 HR(pDevice->CreateTexture2D(&texArrayDesc, 0, &texArray));

	 // for each texture element...
	 for (UINT texElement = 0; texElement < size; ++texElement)
	 {
		 // for each mipmap level...
		 for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		 {
			 D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			 HR(pDC->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			 pDC->UpdateSubresource(texArray,
				 D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			 pDC->Unmap(srcTex[texElement], mipLevel);
		 }
	 }
	 //
	 // Create a resource view to the texture array.
	 //

	 D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	 viewDesc.Format = texArrayDesc.Format;
	 viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	 viewDesc.Texture2DArray.MostDetailedMip = 0;
	 viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	 viewDesc.Texture2DArray.FirstArraySlice = 0;
	 viewDesc.Texture2DArray.ArraySize = size;

	 ID3D11ShaderResourceView* texArraySRV = 0;
	 HR(pDevice->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

	 //
	 // Cleanup--we only need the resource view.
	 //

	 SAFE_RELEASE(texArray);

	 for (UINT i = 0; i < size; ++i)
		 SAFE_RELEASE(srcTex[i]);

	 return texArraySRV;




}



void TextureResourceMGR::InsertTextureView(ID3D11ShaderResourceView* srv, std::wstring nameStr)
{
	
	if (srv == nullptr) return;

	m_ResourceMap.insert({ nameStr, srv });
}

ID3D11ShaderResourceView* TextureResourceMGR::GetTextureMap(std::wstring nameStr)
{
	ID3D11ShaderResourceView* srv = m_ResourceMap[nameStr];

	if (srv != nullptr) 
		return srv;
	else return nullptr;
}


