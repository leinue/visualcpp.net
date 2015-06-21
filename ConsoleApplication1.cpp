// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include "TlHelp32.h"
#include <iostream>
#include "process.h"

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

	TerminateProcess(hNoteHandle,0);
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
		cout << pe32.szExeFile << endl;
		cout << pe32.cntThreads << endl;
		cout << pe32.th32ProcessID << endl;
		cout << "==============================" << endl;
	}

	CloseHandle(hSnapShot);

	system("pause");

	return 0;
}

