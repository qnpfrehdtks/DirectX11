#include <d3d11.h>

#ifndef SKY_H
#define SKY_H

class cCamera;


class cSky
{

public:
	cSky(ID3D11Device* device, const std::wstring& cubemapFilename, float SkyRadius);
	~cSky();

	void BuildSkyGeometry(ID3D11Device * device, float SkyRadius);
	void DrawSky(ID3D11DeviceContext* dc, const cCamera* camera);
	void DrawGbufferSky(ID3D11DeviceContext* dc, const cCamera* camera);
	ID3D11ShaderResourceView* GetCubeMapSRV() { return m_SkySRV; }


private:

	ID3D11Buffer * m_skyVB;
	ID3D11Buffer* m_skyIB;

	ID3D11ShaderResourceView* m_SkySRV;
	UINT m_IdexCount;
};

#endif