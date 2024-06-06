#ifndef UI_COMMON_SKIN_HPP
#define UI_COMMON_SKIN_HPP


namespace DuiLib {

    
    DUILIB_API CDuiString GetCommonBkImage();
    DUILIB_API void SetCommonBkImage(LPCTSTR bkImage);
    DUILIB_API DWORD GetCommonBkColor();
    DUILIB_API void SetCommonBkColor(DWORD bkColor);
    DUILIB_API int GetCommonOpacity();
    DUILIB_API void SetCommonOpacity(int opacity);

    DUILIB_API BOOL UISetCommonSkin(LPCTSTR bkImage, DWORD bkColor, int opacity);

}

#endif // UI_COMMON_SKIN_HPP
