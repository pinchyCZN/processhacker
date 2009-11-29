#ifndef NTIMPORT_H
#define NTIMPORT_H

#include <ntbasic.h>
#include <ntexapi.h>
#include <ntobapi.h>
#include <ntpsapi.h>
#include <ntrtl.h>

#ifdef NTIMPORT_PRIVATE
#define EXT
#define EQNULL = NULL
#else
#define EXT extern
#define EQNULL
#endif

EXT _NtAlertResumeThread NtAlertResumeThread EQNULL;
EXT _NtAlertThread NtAlertThread EQNULL;
EXT _NtClose NtClose EQNULL;
EXT _NtDuplicateObject NtDuplicateObject EQNULL;
EXT _NtGetNextProcess NtGetNextProcess EQNULL;
EXT _NtGetNextThread NtGetNextThread EQNULL;
EXT _NtOpenProcess NtOpenProcess EQNULL;
EXT _NtOpenThread NtOpenThread EQNULL;
EXT _NtQueryInformationProcess NtQueryInformationProcess EQNULL;
EXT _NtQueryInformationThread NtQueryInformationThread EQNULL;
EXT _NtQueryObject NtQueryObject EQNULL;
EXT _NtQuerySystemInformation NtQuerySystemInformation EQNULL;
EXT _NtQueueApcThread NtQueueApcThread EQNULL;
EXT _NtResumeProcess NtResumeProcess EQNULL;
EXT _NtResumeThread NtResumeThread EQNULL; 
EXT _NtSetInformationProcess NtSetInformationProcess EQNULL;
EXT _NtSetInformationThread NtSetInformationThread EQNULL;
EXT _NtSuspendProcess NtSuspendProcess EQNULL;
EXT _NtSuspendThread NtSuspendThread EQNULL;
EXT _NtTerminateProcess NtTerminateProcess EQNULL;
EXT _NtTerminateThread NtTerminateThread EQNULL;

BOOLEAN PhInitializeImports();

#endif