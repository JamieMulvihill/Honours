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
		XMFLOAT4 texel;
	};

public:
	VolumePreservationTestShader(ID3D11Device* device, HWND hwnd, int w, int h);
	~VolumePreservationTestShader();

	void setShaderParameters(ID3D11DeviceContext* dc, /*ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,*/ ID3D11ShaderResourceView* deformationMap, XMFLOAT4& result);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);
	

	float totalVolume = 0.f;
	float currentVolume = 0.f;
private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;

	int mNumElements = 1024;
	std::vector<Data> dataA = std::vector<Data>(mNumElements * mNumElements);
	std::vector<Data> dataB = std::vector<Data>(mNumElements * mNumElements);

	ID3D11ShaderResourceView* mInputASRV;
	ID3D11ShaderResourceView* mInputBSRV;

	ID3D11ShaderResourceView* InputA;
	ID3D11ShaderResourceView* InputB;
	ID3D11UnorderedAccessView* Output;

	ID3D11Buffer* mOutputDebugBuffer;
	ID3D11Buffer* mOutputBuffer;

};