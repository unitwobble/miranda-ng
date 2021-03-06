/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

PlugOptions opts;
POPUP_OPTIONS PopupOptions = { 0 };

static int GetBits(HWND hwndDlg)
{
	return IsDlgButtonChecked(hwndDlg, IDC_CHANGE_PLATFORM) ? DEFAULT_OPP_BITS : DEFAULT_BITS;
}

static int GetUpdateMode()
{
	int UpdateMode = g_plugin.getByte(DB_SETTING_UPDATE_MODE, -1);

	// Check if there is url for custom mode
	if (UpdateMode == UPDATE_MODE_CUSTOM) {
		ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
		if (url == NULL || !wcslen(url)) {
			// No url for custom mode, reset that setting so it will be determined automatically
			g_plugin.delSetting(DB_SETTING_UPDATE_MODE);
			UpdateMode = -1;
		}
	}

	if (UpdateMode < 0 || UpdateMode > UPDATE_MODE_MAX_VALUE) {
		// Missing or unknown mode, determine correct from version of running core
		char coreVersion[512];
		Miranda_GetVersionText(coreVersion, _countof(coreVersion));
		UpdateMode = (strstr(coreVersion, "alpha") == nullptr) ? UPDATE_MODE_STABLE : UPDATE_MODE_TRUNK;
	}

	return UpdateMode;
}

wchar_t* GetDefaultUrl()
{
	wchar_t url[MAX_PATH];
	switch (GetUpdateMode()) {
	case UPDATE_MODE_STABLE:
		mir_snwprintf(url, DEFAULT_UPDATE_URL, opts.bChangePlatform ? DEFAULT_OPP_BITS : DEFAULT_BITS);
		return mir_wstrdup(url);
	case UPDATE_MODE_STABLE_SYMBOLS:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, opts.bChangePlatform ? DEFAULT_OPP_BITS : DEFAULT_BITS);
		return mir_wstrdup(url);
	case UPDATE_MODE_TRUNK:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_TRUNK, opts.bChangePlatform ? DEFAULT_OPP_BITS : DEFAULT_BITS);
		return mir_wstrdup(url);
	case UPDATE_MODE_TRUNK_SYMBOLS:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, opts.bChangePlatform ? DEFAULT_OPP_BITS : DEFAULT_BITS);
		return mir_wstrdup(url);
	default:
		return g_plugin.getWStringA(DB_SETTING_UPDATE_URL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t defurl[MAX_PATH];

	switch (msg) {
	case WM_INITDIALOG:
		if (opts.bUpdateOnStartup) {
			CheckDlgButton(hwndDlg, IDC_UPDATEONSTARTUP, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), TRUE);
		}

		CheckDlgButton(hwndDlg, IDC_ONLYONCEADAY, opts.bOnlyOnceADay ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHANGE_PLATFORM, opts.bChangePlatform ? BST_CHECKED : BST_UNCHECKED);

		if (opts.bUpdateOnPeriod) {
			CheckDlgButton(hwndDlg, IDC_UPDATEONPERIOD, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIOD), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODSPIN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), TRUE);
		}
		CheckDlgButton(hwndDlg, IDC_SILENTMODE, opts.bSilentMode ? BST_CHECKED : BST_UNCHECKED);
		if (g_plugin.getByte(DB_SETTING_NEED_RESTART, 0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_NEEDRESTARTLABEL), SW_SHOW);

		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETPOS, 0, (LPARAM)opts.Period);

		if (ServiceExists(MS_AB_BACKUP)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKUP), TRUE);
			SetDlgItemText(hwndDlg, IDC_BACKUP, LPGENW("Backup database before update"));
			if (opts.bBackup)
				CheckDlgButton(hwndDlg, IDC_BACKUP, BST_CHECKED);
		}

		Edit_LimitText(GetDlgItem(hwndDlg, IDC_PERIOD), 2);

		if (g_plugin.getByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 0)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STABLE), FALSE);
			// Reset setting if needed
			int UpdateMode = g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
			if (UpdateMode == UPDATE_MODE_STABLE)
				g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
			SetDlgItemText(hwndDlg, IDC_STABLE, LPGENW("Stable version (incompatible with current development version)"));
		}
		TranslateDialogDefault(hwndDlg);

		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 0, TranslateT("hours"));
		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 1, TranslateT("days"));
		ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), opts.bPeriodMeasure);

		switch (GetUpdateMode()) {
		case UPDATE_MODE_STABLE:
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			CheckDlgButton(hwndDlg, IDC_STABLE, BST_CHECKED);
			break;
		case UPDATE_MODE_STABLE_SYMBOLS:
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			CheckDlgButton(hwndDlg, IDC_STABLE_SYMBOLS, BST_CHECKED);
			break;
		case UPDATE_MODE_TRUNK:
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			CheckDlgButton(hwndDlg, IDC_TRUNK, BST_CHECKED);
			break;
		case UPDATE_MODE_TRUNK_SYMBOLS:
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			CheckDlgButton(hwndDlg, IDC_TRUNK_SYMBOLS, BST_CHECKED);
			break;
		default:
			CheckDlgButton(hwndDlg, IDC_CUSTOM, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), FALSE);

			ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
			if (url == NULL)
				url = GetDefaultUrl();
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, url);
		}

#ifndef _WIN64
		SetDlgItemText(hwndDlg, IDC_CHANGE_PLATFORM, TranslateT("Change platform to 64-bit"));
		{
			BOOL bIsWow64 = FALSE;
			IsWow64Process(GetCurrentProcess(), &bIsWow64);
			if (!bIsWow64)
				ShowWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), SW_HIDE);
		}
#endif
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UPDATEONSTARTUP:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP));
			__fallthrough;

		case IDC_SILENTMODE:
		case IDC_ONLYONCEADAY:
		case IDC_BACKUP:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_UPDATEONPERIOD:
			{
				BOOL value = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONPERIOD);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PERIOD), value);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODSPIN), value);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), value);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_TRUNK_SYMBOLS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TRUNK:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_STABLE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_STABLE_SYMBOLS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, GetBits(hwndDlg));
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_CUSTOM:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), TRUE);
			{
				ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
				if (url == NULL)
					url = GetDefaultUrl();
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, url);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PERIOD:
		case IDC_CUSTOMURL:
			if ((HWND)lParam == GetFocus() && (HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PERIODMEASURE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_CHANGE_PLATFORM:
			if (IsDlgButtonChecked(hwndDlg, IDC_STABLE)) {
				mir_snwprintf(defurl, DEFAULT_UPDATE_URL, GetBits(hwndDlg));
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_STABLE_SYMBOLS)) {
				mir_snwprintf(defurl, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, GetBits(hwndDlg));
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_TRUNK)) {
				mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK, GetBits(hwndDlg));
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_TRUNK_SYMBOLS)) {
				mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, GetBits(hwndDlg));
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, defurl);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_LINK_HOTKEY:
			g_plugin.openOptions(L"Customize", L"Hotkeys");
			return true;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr && hdr->code == UDN_DELTAPOS)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			if (hdr && hdr->code == PSN_APPLY) {
				g_plugin.setByte("UpdateOnStartup", opts.bUpdateOnStartup = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP));
				g_plugin.setByte("OnlyOnceADay", opts.bOnlyOnceADay = IsDlgButtonChecked(hwndDlg, IDC_ONLYONCEADAY));
				g_plugin.setByte("UpdateOnPeriod", opts.bUpdateOnPeriod = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONPERIOD));
				g_plugin.setByte("PeriodMeasure", opts.bPeriodMeasure = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE)));
				g_plugin.setByte("SilentMode", opts.bSilentMode = IsDlgButtonChecked(hwndDlg, IDC_SILENTMODE));
				g_plugin.setByte("Backup", opts.bBackup = IsDlgButtonChecked(hwndDlg, IDC_BACKUP));
				wchar_t buffer[3] = { 0 };
				Edit_GetText(GetDlgItem(hwndDlg, IDC_PERIOD), buffer, _countof(buffer));
				g_plugin.setDword("Period", opts.Period = _wtoi(buffer));

				InitTimer((void*)1);

				bool bNoSymbols = false;
				if (IsDlgButtonChecked(hwndDlg, IDC_STABLE)) {
					g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
					if (!opts.bChangePlatform)
						opts.bForceRedownload = 0;
					g_plugin.delSetting(DB_SETTING_REDOWNLOAD);
					bNoSymbols = true;
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_STABLE_SYMBOLS)) {
					// Only set ForceRedownload if the previous UpdateMode was different
					// to redownload all plugin with pdb files
					if (g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE) != UPDATE_MODE_STABLE_SYMBOLS) {
						g_plugin.setByte(DB_SETTING_REDOWNLOAD, opts.bForceRedownload = 1);
						g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE_SYMBOLS);
					}
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_TRUNK)) {
					g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
					if (!opts.bChangePlatform)
						opts.bForceRedownload = 0;
					g_plugin.delSetting(DB_SETTING_REDOWNLOAD);
					bNoSymbols = true;
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_TRUNK_SYMBOLS)) {
					// Only set ForceRedownload if the previous UpdateMode was different
					// to redownload all plugin with pdb files
					if (g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE) != UPDATE_MODE_TRUNK_SYMBOLS) {
						g_plugin.setByte(DB_SETTING_REDOWNLOAD, opts.bForceRedownload = 1);
						g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK_SYMBOLS);
					}
				}
				else {
					wchar_t tszUrl[100];
					GetDlgItemText(hwndDlg, IDC_CUSTOMURL, tszUrl, _countof(tszUrl));
					g_plugin.setWString(DB_SETTING_UPDATE_URL, tszUrl);
					g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_CUSTOM);
					opts.bForceRedownload = 0;
					g_plugin.delSetting(DB_SETTING_REDOWNLOAD);
				}

				if (IsDlgButtonChecked(hwndDlg, IDC_CHANGE_PLATFORM)) {
					g_plugin.setByte(DB_SETTING_REDOWNLOAD, opts.bForceRedownload = 1);
					g_plugin.setByte(DB_SETTING_CHANGEPLATFORM, opts.bChangePlatform = 1);
				}
				else g_plugin.setByte(DB_SETTING_CHANGEPLATFORM, opts.bChangePlatform = 0);

				// if user selected update channel without symbols, remove PDBs
				if (bNoSymbols) {
					CMStringW wszPath(VARSW(L"%miranda_path%"));
					wszPath.AppendChar('\\');

					WIN32_FIND_DATA ffd;
					HANDLE hFind = FindFirstFile(wszPath + L"*.pdb", &ffd);
					if (hFind != INVALID_HANDLE_VALUE) {
						do {
							DeleteFileW(wszPath + ffd.cFileName);
						}
							while (FindNextFile(hFind, &ffd) != 0);
					}
					FindClose(hFind);
				}

				// if user tried to change the channel, run the update dialog immediately
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHANGE_PLATFORM)))
					CallService(MS_PU_CHECKUPDATES, 0, 0);
			}
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgPopupOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		//Colors
		if (PopupOptions.DefColors == byCOLOR_OWN) {
			CheckDlgButton(hdlg, IDC_USEOWNCOLORS, BST_CHECKED);
		}
		else if (PopupOptions.DefColors == byCOLOR_WINDOWS) {
			CheckDlgButton(hdlg, IDC_USEWINCOLORS, BST_CHECKED);
		}
		else if (PopupOptions.DefColors == byCOLOR_POPUP) {
			CheckDlgButton(hdlg, IDC_USEPOPUPCOLORS, BST_CHECKED);
		}
		for (int i = 0; i < POPUPS; i++) {
			SendDlgItemMessage(hdlg, (i + 42071), CPM_SETCOLOUR, 0, PopupsList[i].colorBack);
			SendDlgItemMessage(hdlg, (i + 41071), CPM_SETCOLOUR, 0, PopupsList[i].colorText);
			EnableWindow(GetDlgItem(hdlg, (i + 42071)), (PopupOptions.DefColors == byCOLOR_OWN));
			EnableWindow(GetDlgItem(hdlg, (i + 41071)), (PopupOptions.DefColors == byCOLOR_OWN));
		}
		// Timeout
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, -1, 9999);
		SetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, PopupOptions.Timeout, TRUE);

		// Mouse actions
		for (auto &it : PopupActions) {
			SendDlgItemMessage(hdlg, IDC_LC, CB_SETITEMDATA, SendDlgItemMessage(hdlg, IDC_LC, CB_ADDSTRING, 0, (LPARAM)TranslateW(it.Text)), it.Action);
			SendDlgItemMessage(hdlg, IDC_RC, CB_SETITEMDATA, SendDlgItemMessage(hdlg, IDC_RC, CB_ADDSTRING, 0, (LPARAM)TranslateW(it.Text)), it.Action);
		}
		SendDlgItemMessage(hdlg, IDC_LC, CB_SETCURSEL, PopupOptions.LeftClickAction, 0);
		SendDlgItemMessage(hdlg, IDC_RC, CB_SETCURSEL, PopupOptions.RightClickAction, 0);

		//Popups notified
		for (int i = 0; i < POPUPS; i++) {
			char str[20] = { 0 };
			mir_snprintf(str, "Popups%d", i);
			CheckDlgButton(hdlg, (i + 40071), (g_plugin.getByte(str, DEFAULT_POPUP_ENABLED)) ? BST_CHECKED : BST_UNCHECKED);
		}
		return TRUE;

	case WM_COMMAND:
		{
			WORD idCtrl = LOWORD(wParam), wNotifyCode = HIWORD(wParam);
			if (wNotifyCode == CPN_COLOURCHANGED) {
				if (idCtrl > 40070) {
					//It's a color picker change. idCtrl is the control id.
					COLORREF color = SendDlgItemMessage(hdlg, idCtrl, CPM_GETCOLOUR, 0, 0);
					int ctlID = idCtrl;
					if ((ctlID > 41070) && (ctlID < 42070)) //It's 41071 or above => Text color.
						PopupsList[ctlID - 41071].colorText = color;
					else if (ctlID > 42070)//Background color.
						PopupsList[ctlID - 42071].colorBack = color;
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
					return TRUE;
				}
				if (idCtrl == IDC_LC)
					PopupOptions.LeftClickAction = (BYTE)SendDlgItemMessage(hdlg, IDC_LC, CB_GETCURSEL, 0, 0);
				else if (idCtrl == IDC_RC)
					PopupOptions.RightClickAction = (BYTE)SendDlgItemMessage(hdlg, IDC_RC, CB_GETCURSEL, 0, 0);

				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
			switch (idCtrl) {
			case IDC_USEOWNCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				PopupOptions.DefColors = byCOLOR_OWN;

				for (int i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), TRUE); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), TRUE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEWINCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				//Use Windows colors
				PopupOptions.DefColors = byCOLOR_WINDOWS;
				for (int i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), FALSE); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), FALSE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEPOPUPCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				//Use Popup colors
				PopupOptions.DefColors = byCOLOR_POPUP;
				for (int i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), FALSE); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), FALSE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_PREVIEW:
				{//Declarations and initializations
					LPCTSTR Title = TranslateT("Plugin Updater");
					LPCTSTR Text = TranslateT("Test");
					for (int i = 0; i < POPUPS; i++) {
						if (IsDlgButtonChecked(hdlg, i + 40071))
							ShowPopup(Title, Text, i);
					}
				}
				break;

			case IDC_TIMEOUT_VALUE:
			case IDC_MSG_BOXES:
			case IDC_ERRORS:
				EnableWindow(GetDlgItem(hdlg, IDC_ERRORS_MSG), BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_ERRORS));
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_INFO_MESSAGES:
				EnableWindow(GetDlgItem(hdlg, IDC_INFO_MESSAGES_MSG), BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_INFO_MESSAGES));
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_PROGR_DLG:
				EnableWindow(GetDlgItem(hdlg, IDC_PROGR_DLG_MSG), BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_PROGR_DLG));
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_MSG_BOXES_MSG:
			case IDC_ERRORS_MSG:
			case IDC_INFO_MESSAGES_MSG:
			case IDC_PROGR_DLG_MSG:
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_RESET:
			//Restore the options stored in memory.
			LoadOptions();
			InitPopupList();
			return TRUE;

		case PSN_APPLY:
			{
				//Text color
				char szSetting[20] = { 0 };
				DWORD ctlColor = 0;
				for (int i = 0; i < POPUPS; i++) {
					ctlColor = SendDlgItemMessage(hdlg, (i + 42071), CPM_GETCOLOUR, 0, 0);
					PopupsList[i].colorBack = ctlColor;
					mir_snprintf(szSetting, "Popups%iBg", i);
					g_plugin.setDword(szSetting, ctlColor);
					ctlColor = SendDlgItemMessage(hdlg, (i + 41071), CPM_GETCOLOUR, 0, 0);
					PopupsList[i].colorText = ctlColor;
					mir_snprintf(szSetting, "Popups%iTx", i);
					g_plugin.setDword(szSetting, ctlColor);
				}
				//Colors
				g_plugin.setByte("DefColors", PopupOptions.DefColors);
				//Timeout
				PopupOptions.Timeout = GetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, nullptr, TRUE);
				g_plugin.setDword("Timeout", PopupOptions.Timeout);
				//Left mouse click
				g_plugin.setByte("LeftClickAction", PopupOptions.LeftClickAction);
				//Right mouse click
				g_plugin.setByte("RightClickAction", PopupOptions.RightClickAction);
				//Notified popups
				for (int i = 0; i < POPUPS; i++) {
					char str[20] = { 0 };
					mir_snprintf(str, "Popups%d", i);
					g_plugin.setByte(str, (BYTE)(IsDlgButtonChecked(hdlg, (i + 40071))));
				}
				return TRUE;
			} //case PSN_APPLY
		} // switch code
		break; //End WM_NOTIFY
	} //switch message
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_UPDATENOTIFY);
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = LPGENW("Plugin Updater");
	odp.pfnDlgProc = UpdateNotifyOptsProc;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
	odp.szGroup.w = LPGENW("Popups");
	odp.szTitle.w = LPGENW("Plugin Updater");
	odp.pfnDlgProc = DlgPopupOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
