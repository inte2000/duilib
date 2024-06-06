#include "StdAfx.h"
#include "DPI.h"
#include "VersionHelpers.h"
namespace DuiLib
{
	//96 DPI = 100% scaling
	//120 DPI = 125% scaling
	//144 DPI = 150% scaling
	//168 DPI = 175% scaling
	//192 DPI = 200% scaling

	typedef HRESULT (WINAPI *LPSetProcessDpiAwareness)(
		_In_ PROCESS_DPI_AWARENESS value
		);

	typedef HRESULT (WINAPI *LPGetProcessDpiAwareness)(
		_In_  HANDLE                hprocess,
		_Out_ PROCESS_DPI_AWARENESS *value
		);


	typedef HRESULT (WINAPI *LPGetDpiForMonitor)(
		_In_  HMONITOR         hmonitor,
		_In_  MONITOR_DPI_TYPE dpiType,
		_Out_ UINT             *dpiX,
		_Out_ UINT             *dpiY
		);


	CDPI::CDPI()
	{
		//m_nScale = 100;
		int nDpi = CDPI::GetMainMonitorDPI(); //orbit 20230113
		m_nScale = ScaleFromSystemDpi(nDpi);
		m_Awareness = PROCESS_PER_MONITOR_DPI_AWARE;
	}

	int CDPI::GetDPIOfMonitor(HMONITOR hMonitor)
	{
		UINT dpix = 96, dpiy = 96;
		if (IsWindows8Point1OrGreater()) {
			HRESULT  hr = E_FAIL;
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPGetDpiForMonitor GetDpiForMonitor = (LPGetDpiForMonitor)GetProcAddress(hModule, "GetDpiForMonitor");
				if (GetDpiForMonitor != NULL && GetDpiForMonitor(hMonitor,MDT_EFFECTIVE_DPI, &dpix, &dpiy) != S_OK) {
					MessageBox(NULL, _T("GetDpiForMonitor failed"), _T("Notification"), MB_OK);
					return 96;
				}
			}
		}
		else {
			HDC screen = GetDC(0);
			dpix = GetDeviceCaps(screen, LOGPIXELSX);
			ReleaseDC(0, screen);
		}
		return dpix;
	}

	int CDPI::GetDPIOfMonitorNearestToPoint(POINT pt)
	{
		HMONITOR hMonitor;
		hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		return GetDPIOfMonitor(hMonitor);
	}

	int CDPI::GetMainMonitorDPI()
	{
		POINT    pt;
		// Get the DPI for the main monitor
		pt.x = 1;
		pt.y = 1;
		return GetDPIOfMonitorNearestToPoint(pt);
	}

    int CDPI::ScaleFromSystemDpi(int dpi)
    {
        int nScale = dpi * 100 / 96;
		int nScale2 = MulDiv(dpi, 100, 96);
        return nScale;
    }


	PROCESS_DPI_AWARENESS CDPI::GetDPIAwareness()
	{
		if (IsWindows8Point1OrGreater()) {
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPGetProcessDpiAwareness GetProcessDpiAwareness = (LPGetProcessDpiAwareness)GetProcAddress(hModule, "GetProcessDpiAwareness");
				if(GetProcessDpiAwareness != NULL) {
					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
					if(GetProcessDpiAwareness(hProcess, &m_Awareness) == S_OK) {
					}
				}
			}
		}

		return m_Awareness;
	}

	BOOL CDPI::SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness)
	{
		BOOL bRet = FALSE;
		if (IsWindows8Point1OrGreater()) {
			HMODULE hModule =::LoadLibrary(_T("Shcore.dll"));
			if(hModule != NULL) {
				LPSetProcessDpiAwareness SetProcessDpiAwareness = (LPSetProcessDpiAwareness)GetProcAddress(hModule, "SetProcessDpiAwareness");
				if (SetProcessDpiAwareness != NULL && SetProcessDpiAwareness(Awareness) == S_OK) {
					m_Awareness = Awareness;
					bRet = TRUE;
				}
			}
		}
		else {
			m_Awareness = Awareness;
		}
		return bRet;
	}
    
    void DuiLib::CDPI::SetDPI(int nDpi)
    {
        m_nScale = ScaleFromSystemDpi(nDpi);
    }

	int CDPI::GetScale()
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return 100;
		}

        return m_nScale;
	}

}