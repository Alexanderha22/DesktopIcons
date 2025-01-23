#ifndef _GetIcon_Hpp_
#define _GetIcon_Hpp_
#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <atlbase.h>
#include <gdiplus.h>
#include <objidl.h>
//using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include <filesystem>
#include <vector>
#include <filesystem>
#include <windowsx.h>

//#define MAXSPEED 2

enum IconState {
    IconStart,
    IconDefault,
    IconFrozen,
    IconTrashed,
};
double getscale();
struct DesktopIcon {
    double positionx{ 0 };
    double positiony{ 0 };

    double directionx{ 1 };
    double directiony{ 1 };

    double speed{ 0 };

    double originx{ 0 };
    double originy{ 0 };

    HBITMAP bitmap;
    int ICONSIZE{ 0 };

    std::wstring name{ L"" };
    std::wstring path{ L"" };

    IconState state = IconStart;

    bool recycle{ false };
    bool collided{ false };
} ;

int GetBitmap(HBITMAP& bitmap, std::wstring File);

std::vector<DesktopIcon> GetIcons();
#endif // !_GetIcon_Hpp_
