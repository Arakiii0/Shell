/*
Program: Shell Emulator in C language
Author: Araki
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <Windows.h>
#include <Shlobj.h>
#include <tchar.h>
#include <TlHelp32.h>


void home_directory(char input[]);
void get_window_ver();
void get_system_info();
void Read_file(char file[]);
void List_files(char directory_path[]);
char *currentusername();

// Get Running Processes
BOOL GetProcessList();
void printError(TCHAR const* msg);


int main(void){

    // all the builtin function
    char *functions[] = {"exit", "echo", "type", "cd", "pwd", "sysinfo", "clear", "cls", "cat", "ls", "history", "hist", "whoami", "id", "ps"};

    // Implement dynamic memory for history list
    int capacity = 2;
    int size = 0;
    char **history = malloc(capacity * sizeof(char *));
    char *username = currentusername();

    while(true){
        // Print prompt
        printf("%s# ", username);
        fflush(stdout);

        // Wait for user input
        char input[100];
        fgets(input, 100, stdin);

        // Remove newline character
        input[strlen(input) - 1] = '\0'; 

        // Adds command to history
        if (size >= capacity) {
            capacity *= 2;
            history = realloc(history, capacity * sizeof(char *));
        }
        history[size] = malloc((strlen(input) + 1) * sizeof(char));
        strcpy(history[size], input);
        size++;

        // Exit Command
        if (!strcmp(input, "exit")) {
            // Free up th memory used by the history list
            for (int i = 0; i < size; i++) {
                free(history[i]);
            }
            free(history);
            exit(0);
        }

        // Echo Command
        if (!strncmp(input, "echo", strlen("echo"))) {
            printf("%s\n", input + 5);
            continue;
        }

        // Type Command, checks if command is found in shell
        if (!strncmp(input, "type", strlen("type"))) {
            bool found = false;
            for (int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {

                // Checks to see if command is in the list of functions above
                if (!strcmp(input + 5, functions[i])) {
                    printf("%s is a shell builtin\n", input +5);
                    found = true;
                    break;
                }
            }
            if (!found)
                printf("%s: not found\n", input + 5);
            continue;
        }

        // Print Current Directory Command
        if (!strcmp(input, "pwd")) {
            char path[100];
            GetCurrentDirectory(100 ,path);
            printf("%s\n", path);
            continue;
        }

        // Changes the directory
        if (!strncmp(input, "cd", strlen("cd"))) {

            // Changes to user's home directory
            if (!strcmp(input + 3, "~")){
                home_directory(input);
                continue;
            }

            if (SetCurrentDirectory(input + 3) == 0) {
                printf("cd %s: No such file or directory\n", input + 3);
            }
            continue;
        }

        // List the computer's information
        if (!strcmp(input, "sysinfo")) {
            get_window_ver();
            get_system_info();
            continue;
        }

        // Clearing terminal
        if (!strcmp(input, "clear") || !strcmp(input, "cls")) {
            system("cls");
            continue;
        }

        // list contents of file
        if (!strncmp(input, "cat", strlen("cat"))) {
            Read_file(input + 4);
            continue;
        }

        // list files in a directory, accepts arguments
        if (!strncmp(input, "ls", strlen("ls"))) {
            // Try to see if can implement so that we if we ls a space, it still defaults to the current directory
            if (strlen(input) == 2) {
                char path[100];
                GetCurrentDirectory(100 ,path);
                List_files(path);
            } else {
                List_files(input + 3);
            }
            continue;
        }

        // Lists the history of the commands
        if (!strcmp(input, "history") || !strcmp(input, "hist")) {
            for (int i = 0; i < size; i++) {
                printf("%i: %s\n", i+1, history[i]);
            }
            continue;
        }

        // Prints the curent user that is using the shell
        if (!strcmp(input, "whoami") || !strcmp(input, "id")) {
            char* username = getenv("USERNAME");
            printf("%s\n", username);
            continue;
        }

        if (!strcmp(input, "ps")) {
            GetProcessList();
            continue;
        }


        // If command is not found in shell
        printf("%s: command not found\n", input);
    }
    
    return 0;
}



// Function to get the current user' name and computername
char *currentusername() {
    static char computername[100];
    DWORD buffersize = sizeof(computername) / sizeof(computername[0]);
    GetComputerName(computername, &buffersize);
    char* username = getenv("USERNAME");

    strcat(computername, "\\");
    strcat(computername, username);

    return computername;
}


// Function to get the user's home directory and change to it
void home_directory(char input[]) {
     WCHAR homeDir[100];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, homeDir))) {
            SetCurrentDirectoryW(homeDir);
        }
        else {
            printf("cd %s: Home Directory not found\n", input + 3);
        }
}


// Function to get current computer's version
void get_window_ver() {
    DWORD dwVersion = 0; 
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0; 
    DWORD dwBuild = 0;

    dwVersion = GetVersion();
 
    // Get the Windows version.
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.
    if (dwVersion < 0x80000000)              
        dwBuild = (DWORD)(HIWORD(dwVersion));

    printf("Version is %d.%d (%d)\n", 
                dwMajorVersion,
                dwMinorVersion,
                dwBuild);
}


// Function to get the current computer's specifications
void get_system_info() {
    SYSTEM_INFO systeminformation;
    // Get the system information
    GetNativeSystemInfo(&systeminformation);

    printf("Processor Architecture: ");
    switch (systeminformation.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("x64 (AMD or Intel)\n");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            printf("ARM\n");
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            printf("ARM64\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("x86\n");
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            printf("Unknown architecture\n");
            break;
    }

    printf("Number of processors: %u\n", systeminformation.dwNumberOfProcessors);
}


// Function to read contents of file
void Read_file(char file[]) {
    HANDLE hFile = CreateFile(
        file,           // File name
        GENERIC_READ,            // Open for reading
        0,                       // Do not share
        NULL,                    // Default security
        OPEN_EXISTING,           // Open existing file only
        FILE_ATTRIBUTE_NORMAL,   // Normal file
        NULL                     // No template file
    );
    
    // If file is not found
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        printf("%s not found.\n", file);
    }
    else {
        DWORD bytesRead;
        char buffer[1000000];  // Buffer to store the read data
        BOOL success = ReadFile(
            hFile,            // Handle to the file
            buffer,           // Buffer to store data
            sizeof(buffer) - 1, // Number of bytes to read (leaving space for null terminator)
            &bytesRead,       // Number of bytes that were read
            NULL              // No overlapping structure (for asynchronous I/O)
        );

        if (!success) {
            printf("Could not read file (error %lu)\n", GetLastError());
            CloseHandle(hFile);
        }
        else {
            // Process the data
            buffer[bytesRead] = '\0';  // Null-terminate the buffer
            printf("%s\n", buffer);

            // Close the file handle
            CloseHandle(hFile);
        }
    }
}


// Function to list files in a specific directory
void List_files(char directory_path[]) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Specify the directory and file type (e.g., *.* for all files)
    char all_files[] = "\\*.*";
    strcat(directory_path, all_files);

    // Step 1: Find the first file in the directory
    hFind = FindFirstFile(directory_path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFile failed (error %lu)\n", GetLastError());
    } 

    // Step 2: List all the files in the directory
    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("[DIR]  %s\n", findFileData.cFileName); // It's a directory
        } else {
            printf("[FILE] %s\n", findFileData.cFileName); // It's a file
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    // Step 3: Handle the end of the list or errors
    if (GetLastError() != ERROR_NO_MORE_FILES) {
        printf("FindNextFile failed (error %lu)\n", GetLastError());
    }

    // Close the search handle
    FindClose(hFind);
}


// Function to Get Running Process
BOOL GetProcessList() {
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE ) {
        printError( TEXT("CreateToolhelp32Snapshot (of processes)") );
        return( FALSE );
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if( !Process32First( hProcessSnap, &pe32 ) ) {
        printError( TEXT("Process32First") ); // show cause of failure
        CloseHandle( hProcessSnap );          // clean the snapshot object
        return( FALSE );
    }

    // Now walk the snapshot of processes
    _tprintf("Running Processes:");
    _tprintf( TEXT("\n=====================================================\n" ));
    int counter = 0;
    do {
        if (counter < 4) {
            counter++;
            continue;
        }
        _tprintf( TEXT("    %i: %s\n"), counter-3, pe32.szExeFile );
        counter++;
    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );
    return( TRUE );
}


void printError(TCHAR const* msg){
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR* p;

    eNum = GetLastError( );
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, eNum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            sysMsg, 256, NULL );

    // Trim the end of the line and terminate it with a null
    p = sysMsg;
    while ( (*p > 31) || (*p == 9) ) {
        ++p;
    }

    do {
        *p-- = 0;
    } while ( (p >= sysMsg) && ( (*p == '.') || (*p < 33) ) );

    // Display the message
    _tprintf( TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg );
}
