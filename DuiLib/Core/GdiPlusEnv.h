#ifndef __GDIPLUS_ENV_INIT_H__
#define __GDIPLUS_ENV_INIT_H__

#include <windows.h>

namespace DuiLib {

	class DUILIB_API CGdiPlusEnv final
	{
	public:
		CGdiPlusEnv()
		{
			Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_GdiplusStartupInput, NULL); // 加载GDI+接口
		}
		~CGdiPlusEnv()
		{
			Release();
		}
		void Release()
		{
			if (m_gdiplusToken != 0)
			{
				Gdiplus::GdiplusShutdown(m_gdiplusToken);	//  卸载GDI+接口
				m_gdiplusToken = 0;
			}
		}
		CGdiPlusEnv (const CGdiPlusEnv &) = delete;
		CGdiPlusEnv(CGdiPlusEnv &&) = delete;
		CGdiPlusEnv & operator = (const CGdiPlusEnv &) = delete;
		CGdiPlusEnv& operator = (CGdiPlusEnv &&) = delete;
	private:
		ULONG_PTR m_gdiplusToken;
		Gdiplus::GdiplusStartupInput m_GdiplusStartupInput;
	};

}// namespace DuiLib

#endif //ifndef __GDIPLUS_ENV_INIT_H__
