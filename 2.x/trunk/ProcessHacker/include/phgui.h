#ifndef PHGUI_H
#define PHGUI_H

#include <phbase.h>
#include <ph.h>
#include <commctrl.h>
#include "resource.h"

// main

INT WINAPI WinMain(
    __in HINSTANCE hInstance,
    __in HINSTANCE hPrevInstance,
    __in LPSTR lpCmdLine,
    __in INT nCmdShow
    );

INT PhMainMessageLoop();

VOID PhInitializeCommonControls();

VOID PhInitializeFont(
    __in HWND hWnd
    );

BOOLEAN PhInitializeSystem();

ATOM PhRegisterWindowClass();

// guisup

// Controls

VOID FORCEINLINE PhSetControlPosition(
    HWND Handle,
    INT Left,
    INT Top,
    INT Right,
    INT Bottom
    )
{
    SetWindowPos(
        Handle,
        NULL,
        Left,
        Top,
        Right - Left,
        Bottom - Top,
        SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER
        );
}

// List Views

HWND PhCreateListViewControl(
    HWND ParentHandle,
    INT_PTR Id
    );

INT PhAddListViewColumn(
    HWND ListViewHandle,
    INT Index,
    INT DisplayIndex,
    INT SubItemIndex,
    INT Format,
    INT Width,
    PWSTR Text
    );

INT PhAddListViewItem(
    HWND ListViewHandle,
    INT Index,
    PWSTR Text,
    PVOID Param
    );

VOID PhSetListViewSubItem(
    HWND ListViewHandle,
    INT Index,
    INT SubItemIndex,
    PWSTR Text
    );

// Tab Controls

HWND PhCreateTabControl(
    HWND ParentWindowHandle
    );

INT PhAddTabControlTab(
    HWND TabControlHandle,
    INT Index,
    PWSTR Text
    );

// mainwnd

BOOLEAN PhMainWndInitialization(
    __in INT ShowCommand
    );

LRESULT CALLBACK PhMainWndProc(      
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

#endif