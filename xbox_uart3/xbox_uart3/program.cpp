// xbox_uart3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Xinput.h>
#include <iostream>
#include "xbox_uart.h"

#define WIN32_LEAN_AND_MEAN			// Strip the unwanted components
#define MONITOR_GAMEPAD_STATE 1
#define BUTTON_START 16
#define BUTTON_BACK 32

#pragma comment (lib,"XInput.lib")	// Necessary for linker

using namespace std;

void printState(XINPUT_STATE* state); bool usbConnect(LPCWSTR portName, DWORD baudRate);
bool sendState(int pId, XINPUT_STATE* state);
bool receiveState(char *buffer);
void checkState(XINPUT_STATE* state);


LPCWSTR portName = L"COM4:";
HANDLE port;

struct Response
{
	int PortB;
};

int _tmain(int argc, _TCHAR* argv[])
{
	XINPUT_STATE s1, s2, s3, s4;
	DWORD e1, e2, e3, e4;
	char recBuffer[BUFFER_SIZE];
	memset(recBuffer, 0, BUFFER_SIZE);
	port = INVALID_HANDLE_VALUE;
	if (!usbConnect(portName, 115200))
	{
		cout << "WARNING: Couldn't open com port.";
	}

	while (true)
	{
		system("CLS");

		e1 = XInputGetState(0, &s1);
		e2 = XInputGetState(1, &s2);
		e3 = XInputGetState(2, &s3);
		e4 = XInputGetState(3, &s4);

		cout << "Blinx Parsec XInput Forwarder v1.0\n\nConnected Gamepads:\n";
		if (   e1 != ERROR_SUCCESS 
			&& e2 != ERROR_SUCCESS
			&& e3 != ERROR_SUCCESS
			&& e4 != ERROR_SUCCESS)
		{
			cout << "No gamepads connected.\n";
		}

		if (e1 == ERROR_SUCCESS)
		{
			cout << "Player 1: \n";
			checkState(&s1);
			printState(&s1);
			if (!sendState(0, &s1))
			{
				cout << "UPLINK ERROR\n";
				usbConnect(portName, 115200);
			}

		}
		Sleep(9);
		if (e2 == ERROR_SUCCESS)
		{
			cout << "Player 2: \n";
			checkState(&s2);
			printState(&s2);
			//sendState(1, &s2);
			if (!sendState(1, &s2))
			{
				cout << "UPLINK ERROR\n";
			}
		}
		Sleep(9);
		if (e3 == ERROR_SUCCESS)
		{
			cout << "Player 3: \n";
			checkState(&s3);
			printState(&s3);
			if (!sendState(2, &s3))
			{
				cout << "UPLINK ERROR\n";
			}
		}
		Sleep(9);
		if (e4 == ERROR_SUCCESS)
		{
			cout << "Player 4: \n";
			checkState(&s4);
			printState(&s4);
			//sendState(3, &s4);
			if (!sendState(3, &s4))
			{
				cout << "UPLINK ERROR\n";

			}
		}

		if (port == INVALID_HANDLE_VALUE)
		{
			cout << "WARNING: Couldn't connect to the serial relay.\n";
			usbConnect(portName, 115200);
		}

		Sleep(9);
	}

	cout << "Exited.";
	system("PAUSE");
	return 0;
}

void checkState(XINPUT_STATE* state)
{
	if ((((int)(state->Gamepad.wButtons) & (BUTTON_START | BUTTON_BACK)) > 32)
		&& (state->Gamepad.bLeftTrigger != 0 && state->Gamepad.bRightTrigger != 0))
	{
		// disable the back button
		printf("WARN: START and BACK pressed at same time. BACK will be disabled.\n");
		state->Gamepad.wButtons = (int)(state->Gamepad.wButtons) ^ (BUTTON_BACK);
	}
	else
	{
		printf("\n");
	}
}

void printState(XINPUT_STATE* state)
{

	printf("%03i %03i %03i %03i %03i %03i %05i\n",
		state->Gamepad.sThumbLX / 256 + 128,
		state->Gamepad.sThumbLY / 256 + 128,
		state->Gamepad.sThumbRX / 256 + 128,
		state->Gamepad.sThumbRY / 256 + 128,
		state->Gamepad.bLeftTrigger,
		state->Gamepad.bRightTrigger,
		state->Gamepad.wButtons
	);
}

bool receiveState(char *buffer)
{
	DWORD bytesTransmitted = 0;			// Byte counter
	DWORD status = EV_RXCHAR;			// transmission status mask

	memset(buffer, 0, BUFFER_SIZE);		// Clear input buffer

	SetCommMask(port, EV_RXCHAR);		// Set up event mask
	WaitCommEvent(port, &status, 0);	// Listen for RX event

	if (status & EV_RXCHAR)				// If event occured
	{
		DWORD success = 0;
		char c = 0;
		do
		{
			if (!ReadFile(port, &c, 1, &success, NULL))	// Read 1 char
			{
				// If error occured, print the message and exit
				DWORD Errors;
				COMSTAT Status;
				ClearCommError(port, &Errors, &Status);		// Clear errors
				memset(buffer, 0, BUFFER_SIZE);				// Clear input buffer
				return FALSE;
			}
			else
			{
				buffer[bytesTransmitted] = c;			// Add last character
				bytesTransmitted++;					// Increase transm. counter
			}
		} while ((success == 1) && (c != '\n'));			// Repeat until the end of message
	}
	return TRUE;
}

bool sendState(int pId, XINPUT_STATE* state)
{
	char buffer[BUFFER_SIZE];					// Create buffer
	memset(buffer, 0, BUFFER_SIZE);				// Clear buffer
	
	XINPUT_GAMEPAD g = state->Gamepad;

	int LX = g.sThumbLX / 256;
	int LY = g.sThumbLY / 256;
	int RX = g.sThumbRX / 256;
	int RY = g.sThumbRY / 256;

	int LT = g.bLeftTrigger;
	int RT = g.bRightTrigger;

	int buttons = g.wButtons;

	// Write formatted data to buffer
	sprintf(buffer, "%c%03i%03i%03i%03i%03i%03i%05i\n",
		(pId % 10) + 'a',
		(LX + 128 % 256),
		(LY + 128 % 256),
		(RX + 128 % 256),
		(RY + 128 % 256),
		(LT % 256),
		(RT % 256),
		buttons
	);
	DWORD bytesTransmitted;
	printf("Buffer sent: \n");
	printf(buffer);
	cout << "\n";
	if (!WriteFile(port, buffer, strlen(buffer), &bytesTransmitted, NULL))// Send data to UART
	{
		// Error occured
		DWORD Errors;
		COMSTAT Status;
		ClearCommError(port, &Errors, &Status);
		return FALSE;
	}
	else
		return TRUE;
}

bool usbConnect(LPCWSTR portName, DWORD baudRate)
{
	DCB				portDCB;					// _DCB struct for serial configuration
	bool			result = FALSE;				// Return value
	COMMTIMEOUTS	comTOUT;					// Communication timeout
	try {
		port = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
		// Try opening port communication
		if (&port == INVALID_HANDLE_VALUE)
		{
			CloseHandle(&port);
			port = NULL;
			return FALSE;
		}
		// NEW SETTINGS
		portDCB.DCBlength = sizeof(DCB);			// Setup config length
		GetCommState(&port, &portDCB);				// Get default port state
		portDCB.BaudRate = baudRate;				// Set baud rate
		portDCB.fBinary = TRUE;						// Enable Binary mode
		portDCB.fParity = FALSE;					// Disable parity 
		portDCB.fOutxCtsFlow = FALSE;				// No CTS 
		portDCB.fOutxDsrFlow = FALSE;				// No DSR
		portDCB.fDtrControl = DTR_CONTROL_DISABLE;	// No DTR
		portDCB.fDsrSensitivity = FALSE;			// No DSR sensitivity 
		portDCB.fTXContinueOnXoff = TRUE;			// TX on XOFF
		portDCB.fOutX = FALSE;						// No XON/XOFF
		portDCB.fInX = FALSE;						//
		portDCB.fErrorChar = FALSE;					// No error correction
		portDCB.fNull = FALSE;						// Keep NULL values
		portDCB.fRtsControl = RTS_CONTROL_DISABLE;	// Disable RTS
		portDCB.fAbortOnError = FALSE;				// Disable abort-on-error
		portDCB.ByteSize = 8;						// 8-bit frames
		portDCB.Parity = NOPARITY;					// Parity: none
		portDCB.StopBits = ONESTOPBIT;				// StopBits: 1

													// Try reconfiguring COM port
		if (!SetCommState(port, &portDCB))
		{
			CloseHandle(port);
			port = NULL;
			return FALSE;
		}

		/// Communication timeout values
		GetCommTimeouts(port, &comTOUT);
		comTOUT.ReadIntervalTimeout = 1;
		comTOUT.ReadTotalTimeoutMultiplier = 1;
		comTOUT.ReadTotalTimeoutConstant = 1;
		/// Set communication time out values
		SetCommTimeouts(port, &comTOUT);
	}
	catch (int e)
	{
	}

	return TRUE;
}