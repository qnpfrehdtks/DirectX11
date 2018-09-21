#pragma once


#include "DXUT.h"
#include "singletonBase.h"
#include <unordered_map>
#include "DDSTextureLoader.h"


class TextureResourceMGR :public SingletonBase<TextureResourceMGR>
{
	
private:
	std::unordered_map<std::wstring, ID3D11ShaderResourceView* > m_ResourceMap;
	ID3D11Device* m_pDevice;
public:
	TextureResourceMGR();
	~TextureResourceMGR();

public:
	
	ID3D11ShaderResourceView* CreateSRV(ID3D11Device* pDevice  ,std::wstring filename);
	ID3D11ShaderResourceView* CreateSRVFromTGA(ID3D11Device* pDevice, std::wstring filename);
	ID3D11ShaderResourceView* CreateSRVFromWIC(ID3D11Device* pDevice, std::wstring filename);
	ID3D11ShaderResourceView* CreateSRVArrFromDDS(ID3D11Device* pDevice, ID3D11DeviceContext* pDC, std::vector<std::wstring> filenames);

	void InsertTextureView(ID3D11ShaderResourceView* srv, std::wstring nameStr);
	ID3D11ShaderResourceView* GetTextureMap(std::wstring nameStr);
	


};

