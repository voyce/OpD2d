// WaveOutputFile.cpp: implementation of the CWaveOutputFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveOutputFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveOutputFile::CWaveOutputFile()
{

}

CWaveOutputFile::~CWaveOutputFile()
{

}

BOOL CWaveOutputFile::Create(LPCTSTR szFilename, WAVEFORMATEX format)
{
	TCHAR *szExt = PathFindExtension(szFilename);
	if (szExt)
	{
		if (_tcsicmp(szExt, _T(".wav")) != 0)
		{
			TCHAR szMsg[512];
			_stprintf_s(szMsg, _countof(szMsg), 
				"You are using a file extension (%s) that is inconsistent with the file format being used (WAV).\n\n"
				"Are you sure you want to continue?", 
				szExt);
			if (AfxMessageBox(szMsg, MB_YESNO) == IDNO)
				return FALSE;
		}
	}
	HANDLE hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (AfxMessageBox("The output file already exists, continuing will overwrite it.\n\nDo you want to continue and overwrite this file?", MB_YESNO) == IDNO)
			return FALSE;
		CloseHandle(hFile);
	}

	m_hio = mmioOpen((char *)szFilename, NULL, MMIO_ALLOCBUF|MMIO_WRITE|MMIO_CREATE);
	if (m_hio == NULL)
	{
		return FALSE;    // cannot save WAVE file
    }

	// Create the output file RIFF chunk of form type 'WAVE'.
	m_ckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
	m_ckOutRIFF.cksize = 0; 
	m_result = mmioCreateChunk(m_hio, &m_ckOutRIFF, MMIO_CREATERIFF);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	// We are now descended into the 'RIFF' chunk we just created.
	// Now create the 'fmt ' chunk. Since we know the size of this chunk,
	// specify it in the MMCKINFO structure so MMIO doesn't have to seek
	// back and set the chunk size after ascending from the chunk.
	m_ckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
	m_ckOut.cksize = sizeof(WAVEFORMATEX);   // we know the size of this ck.
	m_result = mmioCreateChunk(m_hio, &m_ckOut, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	/* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. */
	if (mmioWrite(m_hio, (HPSTR)&format, sizeof(WAVEFORMATEX))!= sizeof(WAVEFORMATEX))
		return FALSE;

	// Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
	m_result = mmioAscend(m_hio, &m_ckOut, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	m_ckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
	m_ckOut.cksize = 0;
    m_result = mmioCreateChunk(m_hio, &m_ckOut, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	m_result = mmioGetInfo(m_hio, &m_info, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;
	
	return TRUE;
}

BOOL CWaveOutputFile::WriteData(LPBYTE pData, UINT nSize)
{
	//ASSERT( AfxIsValidAddress(pData, nSize, FALSE) );

	for (UINT cT = 0; cT < nSize; ++cT)
    {       
		if (m_info.pchNext == m_info.pchEndWrite)
		{
			m_info.dwFlags |= MMIO_DIRTY;
			m_result = mmioAdvance(m_hio, &m_info, MMIO_WRITE);
			if (m_result != MMSYSERR_NOERROR)
				return FALSE;
		}
		*((BYTE*)m_info.pchNext) = *((BYTE*)pData+cT);
        // careful!! make sure this is only incrementing by one byte!
		(m_info.pchNext)++;
	}
	return TRUE;
}

BOOL CWaveOutputFile::Flush()
{
	m_info.dwFlags |= MMIO_DIRTY;
	m_result = mmioAdvance(m_hio, &m_info, MMIO_WRITE);
	if (m_result != MMSYSERR_NOERROR)
			return FALSE;

	m_result = mmioSetInfo(m_hio, &m_info, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	// Ascend the output file out of the 'data' chunk -- this will cause
	// the chunk size of the 'data' chunk to be written.
	m_result = mmioAscend(m_hio, &m_ckOut, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

	// Do this here instead...
	m_result = mmioAscend(m_hio, &m_ckOutRIFF, 0);
	if (m_result != MMSYSERR_NOERROR)
		return FALSE;

    mmioClose(m_hio, 0);
	m_hio = NULL;
	return TRUE;
}
