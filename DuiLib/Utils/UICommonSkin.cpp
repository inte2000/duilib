#include "stdafx.h"
#include "UICommonSkin.h"

namespace DuiLib {

    CDuiString s_CommonBkImage;
    DWORD s_CommonBkColor = 0;
    int s_CommonOpacity = 255;

    CDuiString GetCommonBkImage()
    {
        return s_CommonBkImage;
    }

    void SetCommonBkImage(LPCTSTR bkImage)
    {
        if ((bkImage != NULL) && _tcslen(bkImage) > 0)
        {
            s_CommonBkImage = bkImage;
            s_CommonBkColor = 0;
        }
    }
    
    DWORD GetCommonBkColor()
    {
        return s_CommonBkColor;
    }

    void SetCommonBkColor(DWORD bkColor)
    {
        if (bkColor != 0)
        {
            s_CommonBkColor = bkColor;
            s_CommonBkImage.Empty();
        }
    }
    
    int GetCommonOpacity()
    {
        return s_CommonOpacity;
    }

    void SetCommonOpacity(int opacity)
    {
        s_CommonOpacity = opacity;
    }

    BOOL UISetCommonSkin(LPCTSTR bkImage, DWORD bkColor, int opacity)
    {
        s_CommonBkImage = bkImage;
        s_CommonBkColor = bkColor;
        s_CommonOpacity = opacity;

        return TRUE;
    }

}