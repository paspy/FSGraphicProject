#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <ctime>
#include "Scenes\GuineaPig.h"

using namespace std;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

#if defined(DEBUG) | defined(_DEBUG)
int main()
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
#endif
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	srand(unsigned int(time(0)));
	GuineaPig GuineaPigApp(GetModuleHandle(NULL));

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	if ( !GuineaPigApp.Init() ) return 0;

	while ( msg.message != WM_QUIT && GuineaPigApp.Run() ) {
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}