/*
* main.cpp - entry point for Linux & Windows Me
*
*/

#include "stdafx.h"
#if defined _WIN32 || _WIN64
#include <windows.h>
#endif

#include "W.h"
#include "PongState.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
	W::createWindow(W::size(800, 600), "WPong");
	W::pushState(new PongState);
	W::start();

	return 0;
}
