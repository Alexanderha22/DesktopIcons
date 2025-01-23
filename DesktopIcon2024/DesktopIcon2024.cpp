// DesktopIcon2024.cpp : Defines the entry point for the application.
//

#include "GetIcon.hpp"

#include "framework.h"
#include "DesktopIcon2024.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

std::vector<DesktopIcon> desktopIcons;
std::vector<Gdiplus::CachedBitmap*> cachedbmp; 
Gdiplus::Bitmap* desktopImage;
Gdiplus::CachedBitmap* desktopCached;
Gdiplus::RectF desktopOffset{ 0, 0, 0, 0 };
bool cached = false;
RECT binRect{ 0,0,0,0 };

POINT MOUSEMOVE { 0, 0 };

POINT windowsize{ 0, 0 };

double MAXSPEED;

std::chrono::system_clock::time_point starttime = std::chrono::system_clock::now();
std::chrono::system_clock::time_point lastframe = std::chrono::system_clock::now();

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void DrawIconsPlus(HDC hdc);


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    HWND                hWnd;
    MSG                 msg;
    WNDCLASS            wndClass;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+.
    desktopIcons = GetIcons();
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Get DesktopImage
    wchar_t path[256];
    SystemParametersInfo(SPI_GETDESKWALLPAPER, 200, &path, 0);
    desktopImage =  Gdiplus::Bitmap::FromFile(path);

    Gdiplus::RectF Deskrect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    Gdiplus::RectF BGrect;
    Gdiplus::SizeF BGsize;
    Gdiplus::Unit t;
    desktopImage->GetBounds(&BGrect, &t);
    BGrect.GetSize(&BGsize);

    desktopOffset.Width = BGrect.Width;
    desktopOffset.Height = BGrect.Height;

    MAXSPEED = Deskrect.Width * ((double)3 / 1920);

    // If image is wider -> higher width/height
    if (BGsize.Width / (double)BGsize.Height > Deskrect.Width / (double)Deskrect.Height)
    {
        desktopOffset.X = (BGrect.Width - (Deskrect.Width / (double)Deskrect.Height * BGrect.Height)) / 2;
        desktopOffset.Width = (Deskrect.Width / (double)Deskrect.Height * BGrect.Height);
    }
    else if (BGsize.Width / (double)BGsize.Height < Deskrect.Width / (double)Deskrect.Height)
    {
        desktopOffset.Y = (BGrect.Height - (Deskrect.Height / (double)Deskrect.Width * BGrect.Width)) / 2;
        desktopOffset.Height = (Deskrect.Height / (double)Deskrect.Width * BGrect.Width);
            //(BGsize.Height / (double)BGsize.Width * GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CYSCREEN));
    }



    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("GettingStarted");

    RegisterClass(&wndClass);

    hWnd = CreateWindow(
        TEXT("GettingStarted"),   // window class name
        TEXT("Getting Started"),  // window caption
        WS_POPUP,
        //WS_OVERLAPPEDWINDOW,      // window style
        CW_USEDEFAULT,            // initial x position
        CW_USEDEFAULT,            // initial y position
        CW_USEDEFAULT,            // initial x size
        CW_USEDEFAULT,            // initial y size
        NULL,                     // parent window handle
        NULL,                     // window menu handle
        hInstance,                // program instance handle
        NULL);                    // creation parameters

    //HWND_TOPMOST
    RECT size;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &size, 0);
    windowsize.x = size.right - size.left;
    windowsize.y = size.bottom - size.top;

    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, windowsize.x, windowsize.y, NULL);
    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    RECT Client_Rect;
    GetClientRect(hWnd, &Client_Rect);
    windowsize.x = Client_Rect.right - Client_Rect.left;
    windowsize.y = Client_Rect.bottom + Client_Rect.left;

    // New start time for speed up
    starttime = std::chrono::system_clock::now();

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (Gdiplus::CachedBitmap* c : cachedbmp) delete c;

    delete desktopImage;
    delete desktopCached;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return msg.wParam;
}  // WinMain

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    HDC          hdc;
    HDC          memdc;
    PAINTSTRUCT  ps;
    HBITMAP      membitmap;
  
    switch (message)
    {
    case WM_PAINT:
    {
        std::chrono::duration<double, std::milli> frametime = std::chrono::system_clock::now() - lastframe;
        if (frametime.count() < (double)1000 / 60)
        {
            return 0;
        }

        lastframe = std::chrono::system_clock::now();

        hdc = BeginPaint(hWnd, &ps);

        memdc = CreateCompatibleDC(hdc);
        membitmap = CreateCompatibleBitmap(hdc, windowsize.x, windowsize.y);
        SelectObject(memdc, membitmap);

        DrawIconsPlus(memdc);

        BitBlt(hdc, 0, 0, windowsize.x, windowsize.y, memdc, 0, 0, SRCCOPY);

        EndPaint(hWnd, &ps);
        
        ReleaseDC(hWnd, memdc);
        ReleaseDC(hWnd, hdc);

        DeleteDC(memdc);
        DeleteDC(hdc);

        DeleteObject(membitmap);
        
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN:
        PostQuitMessage(0);
        return 0;
        /*
    case WM_RBUTTONDOWN:
        for (int i = 0; i < desktopIcons.size(); i++) desktopIcons[i].frozen = !desktopIcons[i].frozen;
        return 0;*/
    case WM_MOUSEMOVE:
        if (MOUSEMOVE.x == 0 && MOUSEMOVE.y == 0)
        {
            MOUSEMOVE.x = GET_X_LPARAM(lParam);
            MOUSEMOVE.y = GET_Y_LPARAM(lParam);
        }
        else if (MOUSEMOVE.x != GET_X_LPARAM(lParam) || MOUSEMOVE.y != GET_Y_LPARAM(lParam))
        {
            PostQuitMessage(0);
        }
        return 0;
    case WM_CHAR:
        switch (wParam)
        {
        case 'f':
        {
            bool allfrozen = true;
            for (int i = 0; i < desktopIcons.size(); i++)
            {
                if (desktopIcons[i].state != IconFrozen)
                {
                    allfrozen = false;
                    desktopIcons[i].state = IconFrozen;
                }
            }
            if (allfrozen)
            {
                for (int i = 0; i < desktopIcons.size(); i++)
                {
                    desktopIcons[i].state = IconStart;
                }
            }
            break;
        }
        case 'w':
            //for (int i = 0; i < desktopIcons.size(); i++) desktopIcons[i].state = IconDefault;
            break;
        case 'r':
            for (int i = 0; i < desktopIcons.size(); i++)
            {
                desktopIcons[i].state = IconFrozen;
                desktopIcons[i].positionx = desktopIcons[i].originx;
                desktopIcons[i].positiony = desktopIcons[i].originy;
            }
            break;
        case 't':
            for (int i = 0; i < desktopIcons.size(); i++)
            {
                // Reset the dirction 
                desktopIcons[i].directionx = 1;
                desktopIcons[i].directiony = 1;

                while (!(abs(desktopIcons[i].directionx) < 0.866 && abs(desktopIcons[i].directionx) > 0.5))
                {
                    double phi = rand() / (double)RAND_MAX * 2 * 3.14;
                    desktopIcons[i].directionx = cos(phi);
                    desktopIcons[i].directiony = sin(phi);
                }

                // Reset state
                desktopIcons[i].state = IconStart;
                desktopIcons[i].speed = 0;
                starttime = std::chrono::system_clock::now();
            }
            break;
        }
    case WM_USER:
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
} // WndProc


void DrawIconsPlus(HDC hdc)
{
    Gdiplus::Graphics graphics(hdc);

    RECT currentRect{ 0,0,0,0 };
    RECT intersect{ 0,0,0,0 };


    if (!cached)
    {
        // BG
        Gdiplus::RectF Deskrect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        graphics.DrawImage(desktopImage, Deskrect,
            desktopOffset.X, desktopOffset.Y, desktopOffset.Width, desktopOffset.Height, Gdiplus::UnitPixel);
        
        Gdiplus::Bitmap* bg = Gdiplus::Bitmap::FromHBITMAP((HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP), (HPALETTE)GetCurrentObject(hdc, OBJ_PAL));
        desktopCached = new Gdiplus::CachedBitmap(bg, &graphics);
        
        // Icons
        for (auto c :  cachedbmp) delete c;
        cachedbmp.clear();

        for (int i = 0; i < desktopIcons.size(); i++)
        {
            BITMAP bmpInfo;
            GetObject(desktopIcons[i].bitmap, sizeof(BITMAP), &bmpInfo);
            int cxBitmap = bmpInfo.bmWidth;
            int cyBitmap = bmpInfo.bmHeight;
            void* bits = bmpInfo.bmBits;

            Gdiplus::Bitmap bmp(cxBitmap, cyBitmap, cxBitmap * 4, PixelFormat32bppARGB, (BYTE*)bits);

            bmp.RotateFlip(Gdiplus::Rotate180FlipX);
            cachedbmp.push_back(new Gdiplus::CachedBitmap(&bmp, &graphics));
        }
        cached = true;
    }

    graphics.DrawCachedBitmap(desktopCached, 0, 0);
    bool allgone = true;

    for (int i = 0; i < desktopIcons.size(); i++)
    {
        // Draw icon
        if (desktopIcons[i].state != IconTrashed)
        {
            if(!desktopIcons[i].recycle) allgone = false;
            if (graphics.DrawCachedBitmap(cachedbmp[i], (INT)desktopIcons[i].positionx, (INT)desktopIcons[i].positiony) != Gdiplus::Ok)
            {
                cached = false;
                break;
            }
        }
        // no need for movement / collision detection
        else continue;

        // Speed up at start
        if (desktopIcons[i].state == IconStart)
        {
            std::chrono::duration<double, std::milli> lerptime = std::chrono::system_clock::now() - starttime;
            desktopIcons[i].speed = std::lerp(0, MAXSPEED, lerptime.count() / 3000);

            if (desktopIcons[i].speed >= MAXSPEED)
            {
                desktopIcons[i].speed = MAXSPEED;
                desktopIcons[i].state = IconDefault;
            }
        }

        // Movement
        if (desktopIcons[i].state == IconDefault || desktopIcons[i].state == IconStart)
        {
            desktopIcons[i].positionx += desktopIcons[i].directionx * desktopIcons[i].speed;
            desktopIcons[i].positiony += desktopIcons[i].directiony * desktopIcons[i].speed;

            if (desktopIcons[i].positionx < 0 || desktopIcons[i].positionx > windowsize.x - desktopIcons[i].ICONSIZE) desktopIcons[i].directionx *= -1;
            if (desktopIcons[i].positiony < 0 || desktopIcons[i].positiony > windowsize.y - desktopIcons[i].ICONSIZE) desktopIcons[i].directiony *= -1;
        }
        
        // Update position for collision detection 
        if (desktopIcons[i].recycle)
        {
            binRect = RECT{ (int)desktopIcons[i].positionx, (int)desktopIcons[i].positiony,
                (int)desktopIcons[i].positionx + desktopIcons[i].ICONSIZE, (int)desktopIcons[i].positiony + desktopIcons[i].ICONSIZE };
        }
        currentRect = RECT{ (int)desktopIcons[i].positionx, (int)desktopIcons[i].positiony,
                (int)desktopIcons[i].positionx + desktopIcons[i].ICONSIZE, (int)desktopIcons[i].positiony + desktopIcons[i].ICONSIZE };


        // Collision
        if (IntersectRect(&intersect, &currentRect, &binRect) && !desktopIcons[i].recycle && !desktopIcons[i].collided)
        {
            if (desktopIcons[i].state == IconDefault)
            {
                desktopIcons[i].speed = desktopIcons[i].speed - (MAXSPEED / 3);
                if (desktopIcons[i].speed <= 0.1) desktopIcons[i].state = IconFrozen;
            }
            else if (desktopIcons[i].state == IconFrozen)
            {
                desktopIcons[i].state = IconTrashed;
            }
            desktopIcons[i].collided = true;
        }
        else if (desktopIcons[i].collided && !IntersectRect(&intersect, &currentRect, &binRect)) desktopIcons[i].collided = false;

    }

    if (allgone)
    {
        // What now lol
        for (int i = 0; i < desktopIcons.size(); i++)
        {
            // Reset position
            desktopIcons[i].positionx = desktopIcons[i].originx;
            desktopIcons[i].positiony = desktopIcons[i].originy;

            // Reset the dirction 
            desktopIcons[i].directionx = 1;
            desktopIcons[i].directiony = 1;

            while (!(abs(desktopIcons[i].directionx) < 0.866 && abs(desktopIcons[i].directionx) > 0.5))
            {
                double phi = rand() / (double)RAND_MAX * 2 * 3.14;
                desktopIcons[i].directionx = cos(phi);
                desktopIcons[i].directiony = sin(phi);
            }
            
            // Reset state
            desktopIcons[i].state = IconStart;
            desktopIcons[i].speed = 0;
            starttime = std::chrono::system_clock::now();
        }
    }
}
