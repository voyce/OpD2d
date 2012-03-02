// Mp3OutputFile.h: interface for the CMp3OutputFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MP3OUTPUTFILE_H__6C31569A_5933_4426_B9D8_111475BB0BB2__INCLUDED_)
#define AFX_MP3OUTPUTFILE_H__6C31569A_5933_4426_B9D8_111475BB0BB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WaveOutputFile.h"
#include "bladedll.h"

class CMp3OutputFile : public COutputFile
{
public:
	CMp3OutputFile();
	virtual ~CMp3OutputFile();

	BOOL Flush();
	BOOL WriteData(LPBYTE pData, UINT nSize);
	BOOL Create(LPCTSTR szFilename, WAVEFORMATEX format);

	USHORT m_uiBitRate;

protected:
	PBYTE m_pMP3Buffer;
	PBYTE m_pBuffer; // overflow buffer
	DWORD m_dwOverflow;
	HBE_STREAM	m_hbeStream;
	HANDLE m_hOut;
	DWORD m_dwSamples;

	BEINITSTREAM	beInitStream;
	BEENCODECHUNK	beEncodeChunk;
	BEDEINITSTREAM	beDeinitStream;
	BECLOSESTREAM	beCloseStream;
	BEVERSION		beVersion;
};

#endif // !defined(AFX_MP3OUTPUTFILE_H__6C31569A_5933_4426_B9D8_111475BB0BB2__INCLUDED_)
