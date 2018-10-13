#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <wingdi.h>
#include <objidl.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

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

int SaveJpeg(HBITMAP hBmp, LPCWSTR lpszFilename, ULONG uQuality)
{
	LPCSTR gdiPlusLib = "Gdiplus.lib";
	HMODULE hModuleThread = LoadLibraryA(gdiPlusLib);

	ULONG *pBitmap = NULL;
	CLSID imageCLSID;
	EncoderParameters encoderParams;
	int iRes = 0;

	typedef Status(WINAPI *pGdipCreateBitmapFromHBITMAP)(HBITMAP, HPALETTE, ULONG**);
	pGdipCreateBitmapFromHBITMAP lGdipCreateBitmapFromHBITMAP;

	typedef Status(WINAPI *pGdipSaveImageToFile)(ULONG *, const WCHAR*, const CLSID*, const EncoderParameters*);
	pGdipSaveImageToFile lGdipSaveImageToFile;

	// load GdipCreateBitmapFromHBITMAP
	lGdipCreateBitmapFromHBITMAP = (pGdipCreateBitmapFromHBITMAP)GetProcAddress(hModuleThread, "GdipCreateBitmapFromHBITMAP");
	if (lGdipCreateBitmapFromHBITMAP == NULL)
	{
		// error
		return 0;
	}

	// load GdipSaveImageToFile
	lGdipSaveImageToFile = (pGdipSaveImageToFile)GetProcAddress(hModuleThread, "GdipSaveImageToFile");
	if (lGdipSaveImageToFile == NULL)
	{
		// error
		return 0;
	}

	lGdipCreateBitmapFromHBITMAP(hBmp, NULL, &pBitmap);

	iRes = GetEncoderClsid(L"image/jpeg", &imageCLSID);
	if (iRes == -1)
	{
		// error
		return 0;
	}
	encoderParams.Count = 1;
	encoderParams.Parameter[0].NumberOfValues = 1;
	encoderParams.Parameter[0].Guid = EncoderQuality;
	encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParams.Parameter[0].Value = &uQuality;

	lGdipSaveImageToFile(pBitmap, lpszFilename, &imageCLSID, &encoderParams);

	return 1;
}

int main()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	LPCWSTR deviceStr = L"DISPLAY";

	// get the device context of the screen
	HDC hScreenDC = CreateDC(deviceStr, NULL, NULL, NULL);
	// and a device context to put it in
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);

	// maybe worth checking these are positive values
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);

	// get a new bitmap
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

	// clean up
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);

	LPCWSTR screenshotFilename = L"C:\\Users\\Alexander Kluth\\Desktop\\test.jpg";
	ULONG jpegQuality = 100;

	SaveJpeg(hBitmap, screenshotFilename, jpegQuality);

	GdiplusShutdown(gdiplusToken);
}