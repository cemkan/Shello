
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <dbghelp.h>

BOOL ListProcessThreads();
void SetTaskPriority(const int par_nThreadID, const int par_nPriority);
void TestFunc(int par_nArg1, bool par_bArg2, short par_sArg3);


#pragma warning(disable : 4996)
void ParseInput(char* par_cpInput)
{
    int npArgs[50] = { 0 };
        
    char* cpSymbolName = strtok(par_cpInput, " ");

    if (cpSymbolName[strlen(cpSymbolName) -1] == '\n')
    {
        cpSymbolName[strlen(cpSymbolName) -1] = '\0';
    }
    
    int nArgCount = 0;
    char* cpArg = 0;
    while (cpArg = strtok(NULL, " "))
    {
        char* cpUnused;
        if ((strlen(cpArg) > 2) && (cpArg[0] == '0') && (cpArg[1] == 'x'))
            npArgs[nArgCount++] = strtol(cpArg, &cpUnused, 16);
        else
            npArgs[nArgCount++] = strtol(cpArg, &cpUnused, 10);
        
    }
    
    printf("Resolved symbol name: %s", cpSymbolName);
    for (int i = 0; i < nArgCount; i++)
        printf(" arg[%d]:%d", i, npArgs[i]);

    if (strncmp(cpSymbolName, "i", 2) == 0)
    {
        ListProcessThreads();
    }
    else if (strncmp(cpSymbolName, "setTaskPriority", 15) == 0)
    {
        SetTaskPriority(npArgs[0], npArgs[1]);
    }
    else
    {
        SymInitialize(GetCurrentProcess(), NULL, TRUE);

        char cpSymbolHolder[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        PSYMBOL_INFO itsSymbol = reinterpret_cast<PSYMBOL_INFO>(cpSymbolHolder);
        itsSymbol->NameLen = MAX_SYM_NAME;
        itsSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        if (SymFromName(GetCurrentProcess(), cpSymbolName, itsSymbol) == TRUE)
        {
            int (*Function)(int, int, int, int, int);
            Function = (int(__cdecl*)(int, int, int, int, int))(itsSymbol->Address);

            int nReturn = Function(npArgs[0], npArgs[1], npArgs[2], npArgs[3], npArgs[4]);
            printf("\n%s returned with %d", cpSymbolName, nReturn);
        }
        else
            printf("\nCould not find function: %s", cpSymbolName);
    }
}




int main()
{
    const int MAX_INPUT_SIZE = 256;
    char cpUserCmd[MAX_INPUT_SIZE];
    while (1)
    {
        printf("->");
        fgets(cpUserCmd, MAX_INPUT_SIZE - 1, stdin);
        if(strlen(cpUserCmd) > 1)
            ParseInput(cpUserCmd);
    }
}


BOOL ListProcessThreads()
{
    DWORD dwOwnerPID = GetCurrentProcessId();
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(hThreadSnap, &te32))
    {
        printf("Thread32First"); // show cause of failure
        CloseHandle(hThreadSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Now walk the thread list of the system,
    // and display information about each thread
    // associated with the specified process

    printf("\nTID\t\tPRI");
    do
    {
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            //_tprintf(TEXT("\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
            //_tprintf(TEXT("     Base priority  = %d"), te32.tpBasePri);
            //_tprintf(TEXT("     Delta priority = %d"), te32.tpDeltaPri);
            printf("\n0x%08X\t%d", te32.th32ThreadID, te32.tpBasePri);
        }
    } while (Thread32Next(hThreadSnap, &te32));

    printf("\n");
    CloseHandle(hThreadSnap);
    return(TRUE);
}

void SetTaskPriority(const int par_nThreadID, const int par_nPriority)
{

    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, par_nThreadID);

    if (hThread != 0)
    {
        SetThreadPriority(hThread, par_nPriority);
        CloseHandle(hThread);
    }
    else
        printf("\nCould not find thread with id 0x%08X", par_nThreadID);
}

void TestFunc(int par_nArg1, bool par_bArg2, short par_sArg3)
{
    printf("\nTest Func called with par_nArg1:%d, bool par_bArg2:%d, short par_sArg3:%d", par_nArg1, par_bArg2, par_sArg3);
}
