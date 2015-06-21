// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include "TlHelp32.h"
#include <iostream>
#include "process.h"
#include "tchar.h"

#define STRLEN 20

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	HWND hNoteWnd = FindWindowA(NULL, "部分API.txt - 记事本");
	if (hNoteWnd == NULL){
		cout << "no this window" << endl;
		cout << hNoteWnd << endl;
	}

	DWORD dwNotePid = 0;
	GetWindowThreadProcessId(hNoteWnd, &dwNotePid);
	if (dwNotePid == 0){
		cout << "no this process id" << endl;
		cout << dwNotePid << endl;
	}
	
	HANDLE hNoteHandle = OpenProcess(PROCESS_ALL_ACCESS,false,dwNotePid);
	if (hNoteHandle == NULL){
		cout << "no this handle" << endl;
		cout << hNoteHandle << endl;
	}

	//TerminateProcess(hNoteHandle,0);
	CloseHandle(hNoteHandle);

	cout << "enum process" << endl;

	HANDLE hSnapShot;
	hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	cout << hSnapShot << endl;
	if (hSnapShot == INVALID_HANDLE_VALUE){
		cout << "调用CeateToolhelp32SnapShot失败" << endl;
	}
	//LPPROCESSENTRY32W pe32 = {0};
	PROCESSENTRY32 pe32 ;
	pe32.dwSize = sizeof(pe32);
	BOOL bRet;
	bRet=Process32First(hSnapShot,&pe32);
	cout << bRet << endl;
	while (bRet){
		bRet = Process32Next(hSnapShot,&pe32);
		_tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
		cout << pe32.cntThreads << endl;
		cout << pe32.th32ProcessID << endl;
		cout << "==============================" << endl;
	}

	CloseHandle(hSnapShot);

	cout << "inject DLL" << endl;

	typedef struct _DATA{
		DWORD dwLoadLIbrary;
		DWORD dwGetProcAddress;
		DWORD dwGetModuleHandle;
		DWORD dwGetModuleFileName;

		char User32Dll[STRLEN];
		char MessageBoxB[STRLEN];
		char Str[STRLEN];
	};

	cout << "inject dll" << endl;
	cout << "please enter pid";
	int dwPid;
	cin >> dwPid;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,false,dwPid);
	if (hProcess == NULL){
		cout << "error in OpenProcess"<<endl;
	}

	cout << "hProcess:" << hProcess << endl;
	_DATA data = { 0 };
	data.dwLoadLIbrary = (DWORD)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"LoadLibraryA");
	data.dwGetProcAddress = (DWORD)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"GetProcAddress");
	data.dwGetModuleHandle = (DWORD)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"GetModuleHandleA");
	data.dwGetModuleFileName = (DWORD)GetProcAddress(GetModuleHandleA("kernel32.dll"),"GetModuleFileNameA");

	lstrcpy(LPWSTR(data.User32Dll),L"user32.dll");
	lstrcpy(LPWSTR(data.MessageBoxB), L"MessageBoxA");
	lstrcpy(LPWSTR(data.Str),L"inject code");

	LPVOID lpData = VirtualAllocEx(hProcess,NULL,sizeof(_DATA),MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
	DWORD dwWriteNum=0;
	WriteProcessMemory(hProcess,lpData,&data,sizeof(_DATA),&dwWriteNum);
	DWORD dwFunSize = 0x2000;
	LPVOID lpCode = VirtualAllocEx(hProcess,NULL,dwFunSize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, lpCode, &data, dwFunSize, &dwWriteNum); 
	HANDLE hRemoteThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)lpCode,lpData,0,NULL);
	WaitForSingleObject(hRemoteThread,INFINITE);
	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);

	system("pause");

	return 0;
}

