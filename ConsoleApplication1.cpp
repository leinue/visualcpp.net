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

// ConsoleApplication2.cpp : 定义控制台应用程序的入口点。
//

//#ifdef TOKEN_H_INCLUDED

//#endif

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKEN_SIZE (100)
#define TOKEN_H_INCLUDED

using namespace std;

typedef enum{
	BAD_TOKEN,
	NUMBER_TOKEN,
	ADD_OPERATOR_TOKEN,
	SUB_OPERATOR_TOKEN,
	MUL_OPERATOR_TOKEN,
	DIV_OPERATOR_TOKEN,
	END_OF_LINE_TOKEN
}TokenKind;

typedef struct{
	TokenKind kind;
	double value;
	char str[MAX_TOKEN_SIZE];
}Token;

void get_line(char *line);
void get_token(Token *token);

static char *st_line;
static int st_line_pos;

typedef enum{
	INITIAL_STATUS,
	IN_INT_PART_STATUS,
	DOT_STATUS,
	IN_FRAC_PART_STATUS
}LexerStatus;

void get_token(Token *token){
	int out_pos = 0;
	LexerStatus status = INITIAL_STATUS;
	char current_char;
	token->kind = BAD_TOKEN;
	while (st_line[st_line_pos] != '\0'){
		current_char = st_line[st_line_pos];
		//printf("current_char=%d", current_char);
		if ((status == IN_INT_PART_STATUS || status == IN_FRAC_PART_STATUS) && isdigit(current_char) && current_char != '.'){
			printf("current_char=%c", current_char);
			token->kind = NUMBER_TOKEN;
			sscanf_s(token->str, "%1f", token->value);
			return;
		}

		if (isspace(current_char)){
			if (current_char == '\r\n'){
				printf("current_char=%c", current_char);
				token->kind = END_OF_LINE_TOKEN;
				return;
			}
			st_line_pos++;
			continue;
		}

		if (out_pos >= MAX_TOKEN_SIZE - 1){
			fprintf(stderr, "token too lang.\r\n");
			exit(1);
		}

		token->str[out_pos] = st_line[st_line_pos];
		st_line_pos++;
		out_pos++;
		token->str[out_pos] = '\0';

		if (current_char == '+'){
			printf("current_char=%c", current_char);
			token->kind = ADD_OPERATOR_TOKEN;
			return;
		}
		else if (current_char == '-'){
			token->kind = SUB_OPERATOR_TOKEN;
			return;
		}
		else if (current_char == '*'){
			token->kind = MUL_OPERATOR_TOKEN;
			return;
		}
		else if (current_char == '/'){
			token->kind = DIV_OPERATOR_TOKEN;
			return;
		}
		else if (isdigit(current_char)){
			if (status == INITIAL_STATUS){
				status = IN_INT_PART_STATUS;
			}
			else if (status == DOT_STATUS){
				status = IN_FRAC_PART_STATUS;
			}
		}
		else if (current_char == '.'){
			if (status == IN_INT_PART_STATUS){
				status = DOT_STATUS;
			}
			else{
				fprintf(stderr, "syntax error.\r\n");
			}
		}
		else{
			fprintf(stderr, "bad character(%c).\r\n", current_char);
			exit(1);
		}

	}
}

void set_line(char *line){
	st_line = line;
	st_line_pos = 0;
}

void parse_line(char *buf){
	Token token;
	set_line(buf);
	for (;;){
		get_token(&token);
		if (token.kind == END_OF_LINE_TOKEN){
			break;
		}
		else{
			//printf("kind..%d,str..%s\r\n",token.kind,token.str);
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{

	char buf[1024];
	while (fgets(buf, 1024, stdin) != NULL){
		printf("receive:%s", buf);
		parse_line(buf);
	}
	return 0;
}


