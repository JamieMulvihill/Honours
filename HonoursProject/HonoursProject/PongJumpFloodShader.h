#pragma once
#include "DXF.h"

using namespace std;
using namespace DirectX;

class PongJumpFloodShader : public BaseShader
{
private: 	
	struct JumpBufferType
	{
		int stepLength;
	};
public:
	PongJumpFloodShader(ID3D11Device* device, HWND hwnd, int w, int h);
	~PongJumpFloodShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* heightMap, int step);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);


private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;

	ID3D11Buffer* jumpBuffer;
};

