//--------------------------------------------------------------------------------------
// https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.hlsl
// this is not the final code, this the code to read from two buffers and add the values
// before writing to a structured buffer, I am trying to read and sum the values from a 
// texture and write to a buffer, using this as guide, attepmting to implement this first
// and adapt and change for programs needs after
//--------------------------------------------------------------------------------------
#include "VolumePreservationTestShader.h"

VolumePreservationTestShader::VolumePreservationTestShader(ID3D11Device* device, HWND hwnd, int w, int h) : BaseShader(device, hwnd)
{
	sWidth = w;
	sHeight = h;
	initShader(L"volumePreservationTest_cs.cso", NULL);

	CreateStructuredBuffer(device, sizeof(BufType), NUM_ELEMENTS, &g_vBuf0[0], &g_pBuf0);
	CreateStructuredBuffer(device, sizeof(BufType), NUM_ELEMENTS, &g_vBuf1[0], &g_pBuf1);
	CreateStructuredBuffer(device, sizeof(BufType), NUM_ELEMENTS, nullptr, &g_pBufResult);

	CreateBufferSRV(device, g_pBuf0, &g_pBuf0SRV);
	CreateBufferSRV(device, g_pBuf1, &g_pBuf1SRV);
	CreateBufferUAV(device, g_pBufResult, &g_pBufResultUAV);
}

VolumePreservationTestShader::~VolumePreservationTestShader()
{

}

void VolumePreservationTestShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile);
	createOutputUAV();

	// populating buffers
	for (int i = 0; i < NUM_ELEMENTS; ++i)
	{
		g_vBuf0[i].i = i;
		g_vBuf0[i].f = (float)i;

		g_vBuf1[i].i = i;
		g_vBuf1[i].f = (float)i;
	}

}

void VolumePreservationTestShader::createOutputUAV()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = sWidth;
	textureDesc.Height = sHeight;
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

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_UNKNOWN; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	descUAV.Buffer.FirstElement = 0;
	descUAV.Buffer.Flags = 0;
	descUAV.Buffer.NumElements = 5;
	renderer->CreateUnorderedAccessView(m_tex, &descUAV, &m_uavAccess);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = 5;
	renderer->CreateShaderResourceView(m_tex, &srvDesc, &m_srvTexOutput);

}


void VolumePreservationTestShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes, ID3D11ShaderResourceView* deformationMap, XMFLOAT4 &result)
{
	dc->CSSetShaderResources(0, 2, aRViews);
	dc->CSSetUnorderedAccessViews(0, 1, &g_pBufResultUAV, nullptr);
	pCBCS = nullptr;
	pCSData = nullptr;

	if (g_pBufResult && resultBuf)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		dc->Map(g_pBufResult, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		memcpy(MappedResource.pData, resultBuf, 0);
		dc->Unmap(g_pBufResult, 0);
		ID3D11Buffer* ppCB[1] = { g_pBufResult };
		dc->CSSetConstantBuffers(0, 1, ppCB);
	}

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

HRESULT VolumePreservationTestShader::CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = nullptr;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
}

HRESULT VolumePreservationTestShader::CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

	return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
}

HRESULT VolumePreservationTestShader::CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

	return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
}

ID3D11Buffer* VolumePreservationTestShader::CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugbuf = nullptr;

	D3D11_BUFFER_DESC desc = {};
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if (SUCCEEDED(pDevice->CreateBuffer(&desc, nullptr, &debugbuf)))
	{
		pd3dImmediateContext->CopyResource(debugbuf, pBuffer);
	}

	return debugbuf;
}

bool VolumePreservationTestShader::ReadFromGPU(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext)
{

	ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(pDevice, pd3dImmediateContext, g_pBufResult);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	BufType* p;
	pd3dImmediateContext->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

	p = (BufType*)MappedResource.pData;

	// Verify that if Compute Shader has done right
	//printf("Verifying against CPU result...");
	bool bSuccess = true;
	for (int i = 0; i < NUM_ELEMENTS; ++i)
		if ((p[i].i != g_vBuf0[i].i + g_vBuf1[i].i) || (p[i].f != g_vBuf0[i].f + g_vBuf1[i].f))
		{
			//printf("failure\n");
			bSuccess = false;

			break;
		}
	if (bSuccess)
		//printf("succeeded\n");

	pd3dImmediateContext->Unmap(debugbuf, 0);

	//SAFE_RELEASE(debugbuf);
	return bSuccess;
}

