// RedPocket.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <stdio.h>
#include <tchar.h>

#define UM_RESET		WM_USER + 1
#define SNATCHER_NUM	4
#define PLAYER_ID		IDC_EDIT1

HWND hTotal;
HWND hPlayer;
CRITICAL_SECTION csRWTotal;
DWORD dwInitTotal = 10000;
DWORD dwSingleSize = 1;

DWORD WINAPI SnatchProc(LPVOID hEdit)
{
	if(hEdit == hPlayer)
	{
		Sleep(30);
	}

	TCHAR szBuffer[10];
	DWORD dwTotal, myRedPocket = 0;
	while(TRUE)
	{
		EnterCriticalSection(&csRWTotal);

		GetWindowText(hTotal, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
		_stscanf(szBuffer, "%d", &dwTotal);
		if(dwTotal < dwSingleSize)
		{
			LeaveCriticalSection(&csRWTotal);
			break;
		}
		_stprintf(szBuffer, "%d", dwTotal - dwSingleSize);
		SetWindowText(hTotal, szBuffer);

		LeaveCriticalSection(&csRWTotal);

		GetWindowText((HWND)hEdit, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
		_stscanf(szBuffer, "%d", &myRedPocket);
		myRedPocket += dwSingleSize;
		_stprintf(szBuffer, "%d", myRedPocket);
		SetWindowText((HWND)hEdit, szBuffer);
	}
	return myRedPocket;
}

DWORD WINAPI ThreadProc(LPVOID hDlg)
{
	InitializeCriticalSection(&csRWTotal);
	
	HANDLE handles[SNATCHER_NUM];
	DWORD IDs[SNATCHER_NUM];
	int i;
	for(i=0; i<SNATCHER_NUM; i++)
	{
		handles[i] = CreateThread(NULL, 0, SnatchProc, (LPVOID)GetDlgItem((HWND)hDlg, IDC_EDIT1 + i), 0, NULL);
		IDs[i] = IDC_EDIT1 + i;
	}
	
	WaitForMultipleObjects(SNATCHER_NUM, (LPHANDLE)handles, TRUE, INFINITE);
	DeleteCriticalSection(&csRWTotal);
	
	DWORD dwPlayerScore = 0, dwNPCScore = 0, dwSumScore = 0, dwRet = 0;
	BOOL bFlag = TRUE;

	for(i=0; i<SNATCHER_NUM; i++)
	{	
		if(IDs[i] == PLAYER_ID)
		{
			GetExitCodeThread(handles[i], &dwPlayerScore);
			dwSumScore = dwPlayerScore;
			break;
		}
	}

	for(i=0; i<SNATCHER_NUM; i++)
	{	
		if(IDs[i] != PLAYER_ID)
		{
			GetExitCodeThread(handles[i], &dwNPCScore);
			dwSumScore += dwNPCScore;
			if(dwNPCScore > dwPlayerScore)
			{
				bFlag = FALSE;
			}
		}
	}

	for(i=0; i<SNATCHER_NUM; i++)
	{
		CloseHandle(handles[i]);
	}

	if(dwSumScore != dwInitTotal)
	{
		MessageBox(0, TEXT("有骇客！！！"), TEXT("提示"), 0);
		dwRet = dwSumScore;
	}
	else if(bFlag != TRUE)
	{
		MessageBox(0, TEXT("你输了！"), TEXT("提示"), 0);
		dwRet = -1;
	}
	else
	{
		MessageBox(0, TEXT("你赢了！"), TEXT("提示"), 0);
		dwRet = 0;
	}
	EnableWindow(GetDlgItem((HWND)hDlg, IDC_BUTTON_RESET), TRUE);
	EnableWindow(GetDlgItem((HWND)hDlg, IDC_BUTTON_START), FALSE);
	return 0;
}

BOOL CALLBACK MainDlgProc(
                HWND hDlg,
                UINT uMsg,
                WPARAM wParam,
                LPARAM lParam)
{
	TCHAR szBuffer[10];
	HANDLE hThread;
	int i;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			hPlayer = GetDlgItem(hDlg, PLAYER_ID);
			hTotal = GetDlgItem(hDlg, IDC_EDIT_TOTAL);
			SendMessage(hDlg, UM_RESET, 0, 0);
			return TRUE;

		case UM_RESET:
			_stprintf(szBuffer, "%d", dwInitTotal);
			SetWindowText(hTotal, szBuffer);

			for(i=0; i<SNATCHER_NUM; i++)
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT1 + i), "0");

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON_START:
					hThread = CreateThread(NULL, 0, ThreadProc, hDlg, 0, NULL);
					CloseHandle(hThread);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RESET), FALSE);
					return TRUE;

				case IDC_BUTTON_RESET:
					SendMessage(hDlg, UM_RESET, 0, 0);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), TRUE);
					return TRUE;

				default:
					return FALSE;
			}
		
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			return TRUE;

		default:
			return FALSE;
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc);
	return 0;
}