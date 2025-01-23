#include "GetIcon.hpp"

int ICONSIZE = 0;

int GetBitmap(HBITMAP &bitmap, std::wstring File)
{
    PCWSTR location = (wchar_t*)File.c_str();

    IShellItemImageFactory* factory;
    SIZE size;
    

    //if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) != S_OK) return -1;
    size.cx = ICONSIZE;
    size.cy = ICONSIZE;

    if (SHCreateItemInKnownFolder(FOLDERID_Desktop, KF_FLAG_DEFAULT, location, IID_IShellItemImageFactory, (void**)&factory) != S_OK) return -2; 

    if (factory->GetImage(size, SIIGBF_THUMBNAILONLY | SIIGBF_RESIZETOFIT, &bitmap) == S_OK)
    {
        factory->Release();
        return 1;
    }
    factory->GetImage(size, SIIGBF_ICONONLY, &bitmap);

    factory->Release();
    return 0;

}


std::vector<DesktopIcon> GetIcons()
{
    srand((unsigned int)time(0));

    // Out Values
    std::vector<DesktopIcon> desktopIcons;
    HBITMAP bitmap;

    // IShellFolder / ItemId
    IShellFolder* Dfolder;
    IEnumIDList* Dlist;
    LPITEMIDLIST Itemid;

    //IFolderView
    IFolderView2* DView;

    IShellWindows* spShellWindows;
    IShellBrowser* spBrowser;
    long lhwnd;

    IShellView* spView;
    IDispatch* spdisp;
    CComVariant vtEmpty;
    CComVariant vtLoc(CSIDL_DESKTOP);
    POINT point;
    STRRET name;

    if (SHGetDesktopFolder(&Dfolder) != S_OK) return desktopIcons;
    if (Dfolder->EnumObjects(NULL, SHCONTF_NONFOLDERS + SHCONTF_FOLDERS, &Dlist) != S_OK) return desktopIcons;


    // GetDesktopFolderView
    if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) != S_OK) return desktopIcons;
    if (CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void**)&spShellWindows) != S_OK) return desktopIcons;
    if (spShellWindows->FindWindowSW(&vtLoc, &vtEmpty, SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp) != S_OK) return desktopIcons;

    CComQIPtr<IServiceProvider>(spdisp)->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&spBrowser));
    spBrowser->QueryActiveShellView(&spView);
    spView->QueryInterface(IID_IFolderView2, (void**)&DView);
    
    FOLDERVIEWMODE temp;
    DView->GetViewModeAndIconSize(&temp, &ICONSIZE);

    double DISPLAYSCALE = getscale();

    int count = 0;
    while (Dlist->Next(1, &Itemid, NULL) == S_OK)
    {
        DView->Item(count, &Itemid);
        DView->GetItemPosition(Itemid, &point);

        Dfolder->GetDisplayNameOf(Itemid, SHGDN_FORPARSING, &name);
        std::filesystem::path path(name.pOleStr);

        

        DesktopIcon current;

        current.path = path.filename().wstring();

        Dfolder->GetDisplayNameOf(Itemid, SHGDN_NORMAL, &name);
        current.name = name.pOleStr;
        current.positionx = point.x / (double)DISPLAYSCALE;
        current.positiony = point.y / (double)DISPLAYSCALE;
        
        current.originx = current.positionx;
        current.originy = current.positiony;

        GetBitmap(bitmap, current.path);
        current.bitmap = bitmap;

        // Random-ish direction
        // Reroll untill it is 'diagonal enough'

        while (!(abs(current.directionx) < 0.866 && abs(current.directionx) > 0.5)) 
        {
            double phi = rand() / (double)RAND_MAX * 2 * 3.14;
            current.directionx = cos(phi);
            current.directiony = sin(phi);
        }

        if (current.name == L"Recycle Bin")
        {
            current.recycle = true;
        }

        current.ICONSIZE = ICONSIZE;

        if (Itemid != NULL) desktopIcons.push_back(current);

        //if (Itemid != NULL) std::wcout << current.path << point.x << '\t' << point.y << '\n';
        count++;
        //if (count == 10) break;
    }

    CoUninitialize();
    return desktopIcons;
}


double getscale()
{
    auto activeWindow = GetActiveWindow();
    HMONITOR monitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

    // Get the logical width 
    MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize = sizeof(monitorInfoEx);
    GetMonitorInfo(monitor, &monitorInfoEx);
    auto cxLogical = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;

    // Get the physical width 
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);
    devMode.dmDriverExtra = 0;
    EnumDisplaySettings(monitorInfoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
    auto cxPhysical = devMode.dmPelsWidth;

    // Calculate the scaling factor
    return ((double)cxPhysical / (double)cxLogical);
}