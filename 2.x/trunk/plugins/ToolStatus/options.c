/*
 * Process Hacker ToolStatus -
 *   rebar code
 *
 * Copyright (C) 2011-2013 dmex
 * Copyright (C) 2010-2013 wj32
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "toolstatus.h"

INT_PTR CALLBACK OptionsDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWND comboHandle = GetDlgItem(hwndDlg, IDC_DISPLAYSTYLECOMBO);

            ComboBox_AddString(comboHandle, L"Images only");
            ComboBox_AddString(comboHandle, L"Selective text");
            ComboBox_AddString(comboHandle, L"All text");
            ComboBox_SetCurSel(comboHandle, PhGetIntegerSetting(L"ProcessHacker.ToolStatus.ToolbarDisplayStyle"));

            Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_TOOLBAR), 
                PhGetIntegerSetting(L"ProcessHacker.ToolStatus.EnableToolBar") ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_SEARCHBOX), 
                PhGetIntegerSetting(L"ProcessHacker.ToolStatus.EnableSearchBox") ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_STATUSBAR), 
                PhGetIntegerSetting(L"ProcessHacker.ToolStatus.EnableStatusBar") ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_RESOLVEGHOSTWINDOWS),
                PhGetIntegerSetting(L"ProcessHacker.ToolStatus.ResolveGhostWindows") ? BST_CHECKED : BST_UNCHECKED);
        }
        break;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
                EndDialog(hwndDlg, IDCANCEL);
                break;
            case IDOK:
                {
                    PhSetIntegerSetting(L"ProcessHacker.ToolStatus.ToolbarDisplayStyle",
                        (DisplayStyle = (TOOLBAR_DISPLAY_STYLE)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_DISPLAYSTYLECOMBO))));
                    PhSetIntegerSetting(L"ProcessHacker.ToolStatus.EnableToolBar",
                        (EnableToolBar = Button_GetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_TOOLBAR)) == BST_CHECKED));
                    PhSetIntegerSetting(L"ProcessHacker.ToolStatus.EnableSearchBox",
                        (EnableSearchBox = Button_GetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_SEARCHBOX)) == BST_CHECKED));
                    PhSetIntegerSetting(L"ProcessHacker.ToolStatus.EnableStatusBar",
                        (EnableStatusBar = Button_GetCheck(GetDlgItem(hwndDlg, IDC_ENABLE_STATUSBAR)) == BST_CHECKED));
                    PhSetIntegerSetting(L"ProcessHacker.ToolStatus.ResolveGhostWindows",
                        Button_GetCheck(GetDlgItem(hwndDlg, IDC_RESOLVEGHOSTWINDOWS)) == BST_CHECKED);

                    LoadToolbarSettings();

                    EndDialog(hwndDlg, IDOK);
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}