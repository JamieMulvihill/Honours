//--------------------------------------------------------------------------------------
// https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.hlsl
//--------------------------------------------------------------------------------------
#include "VolumePreservationTestShader.h"

VolumePreservationTestShader::VolumePreservationTestShader(ID3D11Device* device, HWND hwnd, int w, int h) : BaseShader(device, hwnd)
{
	sWidth = w;
	sHeight = h;
	initShader(L"volumePreservationTest_cs.cso", NULL);
}

VolumePreservationTestShader::~VolumePreservationTestShader()
{
}

void VolumePreservationTestShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile);

	int count = 0;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 256; j++) {
			dataA[count].texel = XMFLOAT4(i, j, 0, j);
			dataB[count].texel = XMFLOAT4(-i, j, i, 0);
			count++;
			
		}
	}
	createOutputUAV();
}

void VolumePreservationTestShader::createOutputUAV()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = mNumElements;
	textureDesc.Height = mNumElements;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_tex = 0;
	renderer->CreateTexture2D(&textureDesc, 0, &m_tex);

	D3D11_BUFFER_DESC inputDesc;
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = sizeof(Data) * mNumElements;
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc.CPUAccessFlags = 0;
	inputDesc.StructureByteStride = sizeof(Data);
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA initDataA;
	initDataA.pSysMem = &dataA[0];
	ID3D11Buffer* bufferA = 0;
	renderer->CreateBuffer(&inputDesc, &initDataA, &bufferA);

	D3D11_SUBRESOURCE_DATA initDataB;
	initDataB.pSysMem = &dataB[0];
	ID3D11Buffer* bufferB = 0;
	renderer->CreateBuffer(&inputDesc, &initDataB, &bufferB);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = mNumElements;
	renderer->CreateShaderResourceView(bufferA, &srvDesc, &mInputASRV);
	renderer->CreateShaderResourceView(bufferB, &srvDesc, &mInputBSRV);

	// Create a read-write buffer the compute shader can
	// write to (D3D11_BIND_UNORDERED_ACCESS).
	D3D11_BUFFER_DESC outputDesc;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ByteWidth = sizeof(Data) * mNumElements;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.StructureByteStride = sizeof(Data);
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	
	renderer->CreateBuffer(&outputDesc, 0, &mOutputBuffer);

	// Create a system memory version of the buffer to read the
	// results back from.
	D3D11_BUFFER_DESC outputBufDesc;
	outputBufDesc.Usage = D3D11_USAGE_STAGING;
	outputBufDesc.BindFlags = 0;
	outputBufDesc.ByteWidth = sizeof(Data) * mNumElements;
	outputBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	outputBufDesc.StructureByteStride = sizeof(Data);
	outputBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	
	renderer->CreateBuffer(&outputBufDesc, 0, &mOutputDebugBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_UNKNOWN; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	descUAV.Buffer.FirstElement = 0;
	descUAV.Buffer.Flags = 0;
	descUAV.Buffer.NumElements = mNumElements;
	renderer->CreateUnorderedAccessView(mOutputBuffer, &descUAV, &m_uavAccess);

}


void VolumePreservationTestShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* deformationMap, XMFLOAT4 &result)
{
	dc->CSSetShaderResources(0, 1, &deformationMap);
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	// Copy the output buffer to system memory.
	dc->CopyResource(mOutputDebugBuffer, mOutputBuffer);
	// Map the data for reading.
	D3D11_MAPPED_SUBRESOURCE mappedData;
	dc->Map(mOutputDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedData);
	Data* dataView = reinterpret_cast<Data*>(mappedData.pData);
	
	for (int i = 0; i < mNumElements; ++i) {
		currentVolume += dataView[i].texel.x;
	}

	dc->Unmap(mOutputDebugBuffer, 0);
	totalVolume = currentVolume;
	currentVolume = 0;
	dataView = nullptr;
}

void VolumePreservationTestShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

