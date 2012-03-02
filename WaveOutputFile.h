// WaveOutputFile.h: interface for the CWaveOutputFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEOUTPUTFILE_H__158A1B25_5E0B_11D3_8876_00105A40DB67__INCLUDED_)
#define AFX_WAVEOUTPUTFILE_H__158A1B25_5E0B_11D3_8876_00105A40DB67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

class COutputFile  
{
public:
	COutputFile() {}
	virtual ~COutputFile() {}

	virtual BOOL Flush() = 0;
	virtual BOOL WriteData(LPBYTE pData, UINT nSize) = 0;
	virtual BOOL Create(LPCTSTR szFilename, WAVEFORMATEX format) = 0;
};

class CWaveOutputFile : public COutputFile
{
public:
	BOOL Flush();
	BOOL WriteData(LPBYTE pData, UINT nSize);
	BOOL Create(LPCTSTR szFilename, WAVEFORMATEX format);
	CWaveOutputFile();
	virtual ~CWaveOutputFile();

protected:
	HMMIO m_hio;
	MMRESULT m_result;
	MMIOINFO m_info;
	MMCKINFO m_ckOut;
	MMCKINFO m_ckOutRIFF;
};


#endif // !defined(AFX_WAVEOUTPUTFILE_H__158A1B25_5E0B_11D3_8876_00105A40DB67__INCLUDED_)
