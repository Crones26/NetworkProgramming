#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include <CommCtrl.h>
#include<cstdio>
#include<iostream>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID PrintInfo(HWND hwnd);
CHAR* IPaddressToString(DWORD dwIPaddress, CHAR sz_IPaddress[]);
CHAR* IPaddressToBinaryString(DWORD dwIPaddress, CHAR sz_IPaddress[]);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hSpin = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
		SendMessage(hSpin, UDM_SETPOS, 0, 1);
		//SetDlgItemInt(hwnd, IDC_EDIT_PREFIX, 1, FALSE);

		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	}
	break;
	case WM_COMMAND:
	{
		HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
		HWND hIPprefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		DWORD dwIPaddress = 0;
		DWORD dwIPmask = 0;
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPaddress);
				if (FIRST_IPADDRESS(dwIPaddress) < 128)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFF000000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 192)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFF0000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 224)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFFFF00);
				PrintInfo(hwnd);
			}
		}
		break;
		case IDC_IPMASK:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
				DWORD count = 0;
				for (DWORD i = dwIPmask; 0x80000000 & i; i <<= 1, count++);
				CHAR szIPprefix[3] = "";
				sprintf(szIPprefix, "%i", count);
				SendMessage(hIPprefix, WM_SETTEXT, 0, (LPARAM)szIPprefix);
			}
			break;

			/*case IDC_EDIT_PREFIX:
			{
				if (HIWORD(wParam) == EN_CHANGE)
				{
					DWORD dwIPmask = UINT_MAX;
					CHAR szIPprefix[3];
					SendMessage(hIPprefix, WM_GETTEXT, 3, (LPARAM)szIPprefix);
					DWORD dwIPprefix = atoi(szIPprefix);
					dwIPmask <<= (32 - dwIPprefix);
					SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPmask);
				}
				break;
			}
			break;*/
		case IDOK:
			break;
		case IDCANCEL:
			EndDialog(hwnd, 0);
		}
		PrintInfo(hwnd);
	}
	break;
	case WM_NOTIFY:
	{
		NMUPDOWN* pNMUpDown = (NMUPDOWN*)lParam;
		if (pNMUpDown->hdr.idFrom == IDC_SPIN_PREFIX)
		{
			if (pNMUpDown->hdr.code == UDN_DELTAPOS)
			{
				HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
				DWORD dwIPmask = UINT_MAX;
				int iPos = pNMUpDown->iPos + pNMUpDown->iDelta;

				if (iPos < 1) iPos = 1;
				if (iPos > 30) iPos = 30;

				dwIPmask <<= (32 - iPos);
				SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPmask);

				CHAR szIPprefix[3];
				sprintf(szIPprefix, "%i", iPos);
				SetDlgItemText(hwnd, IDC_EDIT_PREFIX, szIPprefix);

				return TRUE;
			}
		}
		PrintInfo(hwnd);
	}
	break;
	case WM_CLOSE:
		FreeConsole();
		EndDialog(hwnd, 0);
	}
	return FALSE;
}
VOID PrintInfo(HWND hwnd)
{
	CONST INT SIZE = 1024;
	CHAR sz_info[SIZE]{};
	CHAR sz_buffer[SIZE]{};
	CHAR sz_IPaddress_buffer[SIZE];
	CHAR sz_IPmask_buffer[SIZE];
	CHAR sz_IPaddress_binary[SIZE];
	CHAR sz_IPmask_binary[SIZE];
	CHAR sz_NetworkIP_buffer[SIZE];
	CHAR sz_BroadcastIP_buffer[SIZE];
	CHAR sz_NumberOfIPs[SIZE];
	CHAR sz_NumberOfHosts[SIZE];
	CHAR sz_prefix[3];
	HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS);
	HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
	HWND hEditPrefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
	HWND hStaticInfo = GetDlgItem(hwnd, IDC_STATIC_INFO);
	DWORD dwIPaddress = 0;
	DWORD dwIPmask = 0;
	DWORD dwIPprefix = 0;

	SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPaddress);
	SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
	SendMessage(hEditPrefix, WM_GETTEXT, 3, (LPARAM)sz_prefix);
	dwIPprefix = atoi(sz_prefix);

	sprintf(sz_IPaddress_buffer, "IP-адрес:\t\t\t %s", IPaddressToString(dwIPaddress, sz_buffer));
	sprintf(sz_IPmask_buffer, "Маска подсети:\t\t %s", IPaddressToString(dwIPmask, sz_buffer));
	sprintf(sz_IPaddress_binary, "IP-адрес (binary):\t\t %s", IPaddressToBinaryString(dwIPaddress, sz_buffer));
	sprintf(sz_IPmask_binary, "Маска (binary):\t\t\t %s", IPaddressToBinaryString(dwIPmask, sz_buffer));
	sprintf(sz_NetworkIP_buffer, "Адрес сети:\t\t\t %s", IPaddressToString(dwIPaddress & dwIPmask, sz_buffer));
	sprintf(sz_BroadcastIP_buffer, "Широковещательный адрес:\t %s", IPaddressToString(dwIPaddress | ~dwIPmask, sz_buffer));
	sprintf(sz_NumberOfIPs, "Количество IP-адресов:\t %u", 1 << (32 - dwIPprefix));
	sprintf(sz_NumberOfHosts, "Количество узлов:\t\t %u", (1 << (32 - dwIPprefix)) - 2);

	sprintf
	(
		sz_info, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",
		sz_IPaddress_buffer, sz_IPaddress_binary,
		sz_IPmask_buffer, sz_IPmask_binary,
		sz_NetworkIP_buffer, sz_BroadcastIP_buffer,
		sz_NumberOfIPs, sz_NumberOfHosts
	);
	SendMessage(hStaticInfo, WM_SETTEXT, 0, (LPARAM)sz_info);
}
CHAR* IPaddressToBinaryString(DWORD dwIPaddress, CHAR sz_IPaddress[])
{
	CHAR sz_buffer[33] = "";
	for (int i = 31; i >= 0; i--)
	{
		sz_buffer[31 - i] = (dwIPaddress & (1 << i)) ? '1' : '0';
		if (i % 8 == 0 && i != 0) sz_buffer[31 - i] = ' ';
	}
	sz_buffer[32] = '\0';
	sprintf(sz_IPaddress, "%s", sz_buffer);
	return sz_IPaddress;
}
CHAR* IPaddressToString(DWORD dwIPaddress, CHAR sz_IPaddress[])
{
	sprintf
	(
		sz_IPaddress,
		"%i.%i.%i.%i",
		FIRST_IPADDRESS(dwIPaddress),
		SECOND_IPADDRESS(dwIPaddress),
		THIRD_IPADDRESS(dwIPaddress),
		FOURTH_IPADDRESS(dwIPaddress)
	);
	return sz_IPaddress;
}