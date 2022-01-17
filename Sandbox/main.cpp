#include <iostream>
#include <helloWorld.h>
#include <windows.h>

int CALLBACK
WinMain(HINSTANCE hInstance, 
        HINSTANCE prevInstance,
		LPSTR lpCmdLine, 
        int showCmd)
{

    MessageBoxA(0, "DX12APP", "DX12APP", MB_OK|MB_ICONINFORMATION);

    return 0;
}