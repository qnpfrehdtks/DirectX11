
#include "DXUT.h"
#include "LightMGR.h"
using namespace DirectX;



class cCamera;
class cGBuffer;
class cShadowMap;
class cPointLightShadowMap;


class TileDeferredingShading
{

public:
	TileDeferredingShading();
	~TileDeferredingShading();

	void Init(ID3D11Device* device, ID3D11DeviceContext* pDC);

	void BuildTextureView(ID3D11Device* device, ID3D11DeviceContext* pDC);
	void BuildStructureBuffer(ID3D11Device* device, ID3D11DeviceContext* pDC);
	void LightPass(ID3D11DeviceContext* pDC, cGBuffer* Gbuffer, cCamera* cam, LPCSTR str, cShadowMap* shadowMap, cPointLightShadowMap* ptShadowMap, ID3D11ShaderResourceView* ssaoMap, CXMMATRIX vp, int mode);
	void BuildScreenQuad(ID3D11Device* pDevice);

	void LightUpdate(ID3D11Device* device, ID3D11DeviceContext* pDC);

	ID3D11ShaderResourceView* GetResultSRV()  { return m_ResultSRV; }

private:
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

	ID3D11UnorderedAccessView* m_ResultUAV;
	ID3D11ShaderResourceView* m_ResultSRV;

	ID3D11ShaderResourceView* m_LightSRV;
	ID3D11ShaderResourceView* m_DirectLightSRV;

	UINT m_ThreadX;
	UINT m_ThreadY;

	ID3D11Buffer* m_PtLightBuffer;
	ID3D11Buffer* m_DirectLightBuffer;

	std::vector<sPtLight> *m_PtLights;
	std::vector<sDirLight> *m_DirLights;

	float A;
	float B;

//	sPtLight* vec[800];

};

