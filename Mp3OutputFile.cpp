// Mp3OutputFile.cpp: implementation of the CMp3OutputFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpD2d.h"
#include "Mp3OutputFile.h"
#include <shlwapi.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMp3OutputFile::CMp3OutputFile() :
	m_hOut(NULL), m_pMP3Buffer(NULL),
		beVersion(NULL)
{
	char szPath[MAX_PATH] = "bladeenc.dll";
	
	HINSTANCE hBladeDLL = LoadLibrary(szPath);
	if(!hBladeDLL)	
	{
		char szErr[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
			GetLastError(), 0, szErr, sizeof(szErr), NULL);
		
		CString str;
		str.Format("Could not load MP3 encoder DLL %s.\n"
			"%s\nPlease refer to http://www.opcode.co.uk/OpD2d for more information",
			szPath, szErr);
		AfxMessageBox( str );
		return;
	}

	// Get Interface
	beInitStream	= (BEINITSTREAM) GetProcAddress(hBladeDLL, TEXT_BEINITSTREAM);
	beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(hBladeDLL, TEXT_BEENCODECHUNK);
	beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(hBladeDLL, TEXT_BEDEINITSTREAM);
	beCloseStream	= (BECLOSESTREAM) GetProcAddress(hBladeDLL, TEXT_BECLOSESTREAM);
	beVersion		= (BEVERSION) GetProcAddress(hBladeDLL, TEXT_BEVERSION);

	if(!beInitStream || !beEncodeChunk || !beDeinitStream || !beCloseStream || !beVersion)	
	{
		AfxMessageBox("Unable to get BladeEnc interface");
	}

	// Get Version
	BE_VERSION	Version;
	beVersion(&Version);

	printf( "BladeEnc.dll version %u.%02u (%u/%u/%u)\n"
			"BladeEnc Engine %u.%02u\n"
			"BladeEnc homepage at %s\n\n",	
			Version.byDLLMajorVersion, Version.byDLLMinorVersion,
			Version.byDay, Version.byMonth, Version.wYear,
			Version.byMajorVersion, Version.byMinorVersion,
			Version.zHomepage);
}

CMp3OutputFile::~CMp3OutputFile()
{
	if (m_hOut != NULL) // didn't flush?
	{
		Flush();
	}
}

BOOL CMp3OutputFile::Flush()
{
	DWORD towrite = 0L;
	BE_ERR		err;
	err = beDeinitStream(m_hbeStream, m_pMP3Buffer, &towrite);
	if(err != BE_ERR_SUCCESSFUL)	
	{
		beCloseStream(m_hbeStream);
		CString str;
		str.Format("beDeinitStream failed (%lu)", err);
		AfxMessageBox(str);
		return FALSE;
	}
	DWORD dwWritten = 0L;
	if (!WriteFile(m_hOut, m_pMP3Buffer, towrite, &dwWritten, NULL))
	{
		CString str;
		str.Format("Failed to write data (%lu)", GetLastError());
		AfxMessageBox(str);
		return FALSE;
	}
	CloseHandle(m_hOut);
	m_hOut = NULL;
	
	err = beCloseStream(m_hbeStream);
	if(err != BE_ERR_SUCCESSFUL)	
	{
		CString str;
		str.Format("beCloseStream failed (%lu)", err);
		AfxMessageBox(str);
		return FALSE;
	}
	return TRUE;
}

BOOL CMp3OutputFile::WriteData(LPBYTE pData, UINT nSize)
{
	try
	{
		DWORD towrite = 0L;
		DWORD dwWritten = 0;
		while (dwWritten < nSize)
		{
			if (nSize-dwWritten < (m_dwSamples*2))
			{
				//TRACE1("%ld bytes overflow\n", nSize-dwWritten);
				
				memcpy(m_pBuffer, pData+dwWritten, nSize-dwWritten);
				m_dwOverflow = nSize-dwWritten;
				break;
			}
			else
			{
				BE_ERR err;
				if (m_dwOverflow)
				{
					// Take first bit of new buffer and add it to the overflow buffer
					//TRACE1("%ld bytes to complete buffer\n", (m_dwSamples*2)-m_dwOverflow);
					
					memcpy(m_pBuffer+m_dwOverflow, pData, (m_dwSamples*2)-m_dwOverflow);
					err = beEncodeChunk(m_hbeStream, m_dwSamples, (short *)m_pBuffer, m_pMP3Buffer, &towrite);
					dwWritten += (m_dwSamples*2)-m_dwOverflow;
					m_dwOverflow = 0;
				}
				else
				{
					err = beEncodeChunk(m_hbeStream, m_dwSamples, (short *)(pData+dwWritten), m_pMP3Buffer, &towrite);
					dwWritten += m_dwSamples*2;
				}
				if(err != BE_ERR_SUCCESSFUL)	
				{
					CString str;
					str.Format("beEncodeChunk failed (%lu)", err);
					AfxMessageBox(str);
					return FALSE;
				}
				DWORD dwCheck = 0L;
				if (!WriteFile(m_hOut, m_pMP3Buffer, towrite, &dwCheck, NULL))
				{
					CString str;
					str.Format("Failed to write data (%lu)", GetLastError());
					AfxMessageBox(str);
					return FALSE;
				}
				//TRACE1("%ld\n", dwWritten);
			}
		}
	}
	catch(...)
	{
		AfxMessageBox("An unhandled exception occurred while writing data.");
	}
	return TRUE;
}

BOOL CMp3OutputFile::Create(LPCTSTR szFilename, WAVEFORMATEX format)
{
	if (!beVersion)
		return FALSE;

	TCHAR *szExt = PathFindExtension(szFilename);
	if (szExt)
	{
		if (_tcsicmp(szExt, _T(".mp3")) != 0)
		{
			TCHAR szMsg[512];
			_stprintf_s(szMsg, _countof(szMsg), 
				"You are using a file extension (%s) that is inconsistent with the file format being used (MP3).\n\n"
				"Are you sure you want to continue?", 
				szExt);
			if (AfxMessageBox(szMsg, MB_YESNO) == IDNO)
				return FALSE;
		}
	}
	BE_CONFIG	beConfig;

	beConfig.dwConfig = BE_CONFIG_MP3;

	beConfig.format.mp3.dwSampleRate	= format.nSamplesPerSec;
	beConfig.format.mp3.byMode			= BE_MP3_MODE_STEREO;
	beConfig.format.mp3.wBitrate		= m_uiBitRate;
	beConfig.format.mp3.bCopyright		= FALSE;
	beConfig.format.mp3.bCRC			= FALSE;
	beConfig.format.mp3.bOriginal		= FALSE;
	beConfig.format.mp3.bPrivate		= FALSE;

	DWORD		dwMP3Buffer;
	BE_ERR		err;

	err = beInitStream(&beConfig, &m_dwSamples, &dwMP3Buffer, &m_hbeStream);
	if(err != BE_ERR_SUCCESSFUL)	
	{
		CString str;
		str.Format("Error opening encoding stream (%lu)", err);
		AfxMessageBox(str);
		return FALSE;
	}

	// Allocate buffers
	m_pMP3Buffer = new BYTE[dwMP3Buffer];
	if(!m_pMP3Buffer)	
	{
		AfxMessageBox("Out of memory");
		err = beCloseStream(m_hbeStream);
		return FALSE;
	}
	m_pBuffer = new BYTE[m_dwSamples*2];
	if(!m_pBuffer)	
	{
		AfxMessageBox("Out of memory");
		err = beCloseStream(m_hbeStream);
		return FALSE;
	}

	m_hOut = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (m_hOut == INVALID_HANDLE_VALUE)
	{
		char szErr[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
			GetLastError(), 0, szErr, sizeof(szErr), NULL);

		CString str;
		str.Format("Failed to create output file.\n%s", szErr);
		AfxMessageBox(str);
		
		err = beCloseStream(m_hbeStream);
		return FALSE;
	}
	m_dwOverflow = 0;
	return TRUE;
}
