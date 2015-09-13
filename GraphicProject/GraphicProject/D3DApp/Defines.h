#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "DirectXTK/DDSTextureLoader.h"
#include "TinyObjLoader/tiny_obj_loader.h"

using namespace DirectX;
using namespace std;
using namespace tinyobj;

// Convenience macro for releasing objects.
#define SafeRelease(x) { if(x){ x->Release(); x = nullptr; } }

// Convenience macro for deleting objects.
#define SafeDelete(x) { delete x; x = nullptr; }

// d3d error checker
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) {												\
		HRESULT hr = (x);									\
		if(FAILED(hr)) {									\
			LPWSTR output;									\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
				FORMAT_MESSAGE_IGNORE_INSERTS 	 |			\
				FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
				NULL,										\
				hr,											\
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
				(LPTSTR) &output,							\
				0,											\
				NULL);										\
			MessageBox(NULL, output, L"Error", MB_OK);		\
		}													\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#define BACKBUFFER_WIDTH	1366
#define BACKBUFFER_HEIGHT	768

#define CAMERA_SPEED 20.0f

#define IDR_MENU1                       101
#define IDD_DIALOG1                     102
#define IDR_ACCELERATOR1                105
#define IDI_ICON1                       106
#define IDC_RENDER_WINDOW               1001
#define IDC_TAB1                        1006
#define IDC_SYSLINK1                    1007
#define IDC_TOOL_DIVIDER                1009
#define IDC_BUTTON2                     1011
#define IDC_BUTTON1                     1012
#define IDC_BUTTON3                     1014
#define IDC_LIST1                       1016
#define IDC_STATIC_FPS                  1017
#define ID_FILE_OPEN_MENU               40001
#define ID_FILE_EXIT                    40002
#define ID_ACCELERATOR_OPEN             40005

#define VK_LW 0x57
#define VK_LS 0x53
#define VK_LA 0x41
#define VK_LD 0x44
