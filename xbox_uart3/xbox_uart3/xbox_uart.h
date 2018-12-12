#include "stdafx.h"
#include <Windows.h>
#include <XInput.h>
#include <math.h>

#pragma comment (lib,"XInput.lib")	// Necessary for linker

#define BUFFER_SIZE 32 // Message size

class XBoxUart
{

public :
	int				controllerID;
	XINPUT_STATE	currentState;
	HANDLE			port;
	XBoxUart();
	XBoxUart(int id);
	bool			isOn();
	bool			connect(LPCWSTR portName, DWORD baudRate);
	bool			disconnect();
	bool			send();
	float			abs(float f);
	bool			receive(char *buffer);
	bool			updateState();
};