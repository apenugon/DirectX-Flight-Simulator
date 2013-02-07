//based on microsoft's scan class

#ifndef _SCANCS_H
#define _SCANCS_H
#include "DXUT.h"


class CScanCS
{
public:
	CScanCS();
	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice );
	void OnD3D11DestroyDevice();


	HRESULT ScanCS( ID3D11DeviceContext* pd3dImmediateContext,

		INT nNumToScan,


		// SRV and UAV of the buffer which contains the input data,
                    // and the scanned result when the function returns
		ID3D11ShaderResourceView* p0SRV,
		ID3D11UnorderedAccessView* p0UAV,

		// SRV and UAV of an aux buffer, which must be the same size as the input/output buffer
		ID3D11ShaderResourceView* p1SRV,
		ID3D11UnorderedAccessView* p1UAV );

private:
	ID3D11ComputeShader* m_pScanCS;
	ID3D11ComputeShader* m_pScan2CS;
	ID3D11ComputeShader* m_pScan3CS;
	ID3D11Buffer* m_pcbCS;

	ID3D11Buffer* m_pAuxBuf;
	ID3D11ShaderResourceView* m_pAuxBufRV;
	ID3D11UnorderedAccessView* m_pAuxBufUAV;

	struct CB_CS
	{
		UINT param[4];
	};
};

#endif