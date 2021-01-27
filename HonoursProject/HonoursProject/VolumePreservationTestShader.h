//--------------------------------------------------------------------------------------
// https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.hlsl
// this is not the final code, this the code to read from two buffers and add the values
// before writing to a structured buffer, I am trying to read and sum the values from a 
// texture and write to a buffer, using this as guide, attepmting to implement this first
// and adapt and change for programs needs after
//--------------------------------------------------------------------------------------


#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;
const UINT NUM_ELEMENTS = 1024;
class VolumePreservationTestShader : public BaseShader
{
private:
	struct Data
	{
		XMFLOAT4 result;
	};

	struct BufType
	{
		int i;
		float f;
	};

public:


	VolumePreservationTestShader(ID3D11Device* device, HWND hwnd, int w, int h);
	~VolumePreservationTestShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes, ID3D11ShaderResourceView* deformationMap, XMFLOAT4& result);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);
	HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut);
	HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut);
	HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut);
	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer);
	bool ReadFromGPU(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext);

	/*bool CreateTextureBuffer(ID3D11DeviceContext* deviceContext);
	bool CreateOutputBuffer(ID3D11DeviceContext* deviceContext);
	bool SettingTextureValues(ID3D11DeviceContext* deviceContext);

	byte* ComputeOutput(ID3D11DeviceContext* deviceContext);
	ID3D11Texture2D* GetOutputTexture() { return outputTexture; }
	ID3D11Texture2D* GetSourceTexture() { return m_tex; }
	UINT GetTextureDataSize() { return m_texDataSize; }
	ID3D11UnorderedAccessView* GetUAV() { return outputUAV; }
	ID3D11ShaderResourceView* GetOutputSRV() { return outputTextureSRV; }*/
private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
	ID3D11UnorderedAccessView* uavOutput;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;


	ID3D11Buffer* g_pBuf0 = nullptr;
	ID3D11Buffer* g_pBuf1 = nullptr;
	ID3D11Buffer* g_pBufResult = nullptr;

	ID3D11ShaderResourceView* g_pBuf0SRV = nullptr;
	ID3D11ShaderResourceView* g_pBuf1SRV = nullptr;
	ID3D11UnorderedAccessView* g_pBufResultUAV = nullptr;

	BufType g_vBuf0[NUM_ELEMENTS];
	BufType g_vBuf1[NUM_ELEMENTS];
	BufType resultBuf[NUM_ELEMENTS];

	ID3D11ShaderResourceView* aRViews[2] = { g_pBuf0SRV, g_pBuf1SRV };
};