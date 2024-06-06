#pragma once
#include <windows.h>
#include "..\..\DuiLib\UIlib.h"

/*
#ifdef UILIB_EXPORTS
#define DUIWEB2API	__declspec(dllexport)
#else
#define DUIWEB2API	__declspec(dllimport)
#endif
*/
#ifdef __cplusplus
extern "C" {
#endif

	DUILIB_API DuiLib::CControlUI* CreateControl(LPCTSTR pstrType);

#ifdef __cplusplus
}
#endif