#ifndef __DPI_H__
#define __DPI_H__
#pragma once

#ifndef DPI_ENUMS_DECLARED

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum MONITOR_DPI_TYPE {
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

#define DPI_ENUMS_DECLARED
#endif // (DPI_ENUMS_DECLARED)

namespace DuiLib
{
	class DUILIB_API CDPI
	{
	public:
		CDPI(void);

	public:
		static int GetMainMonitorDPI();
		static int GetDPIOfMonitor(HMONITOR hMonitor);
		static int GetDPIOfMonitorNearestToPoint(POINT pt);
        static int ScaleFromSystemDpi(int dpi);
	public:
		PROCESS_DPI_AWARENESS GetDPIAwareness();
		BOOL SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness);
		int GetDPI();
        void SetDPI(int nDpi);

		int GetScale();

	private:
		int m_nScale;
		PROCESS_DPI_AWARENESS m_Awareness;
	};
}
#endif //__DPI_H__