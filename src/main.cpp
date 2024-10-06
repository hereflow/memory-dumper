#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>

void dump_process_memory( DWORD pid, const std::wstring &dumpFile )
{
    HANDLE process = OpenProcess( PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid );
    if ( process == nullptr )
    {
        return;
    }

    std::wofstream dump( dumpFile );
    if ( !dump.is_open( ) )
    {
        CloseHandle( process );
        return;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo( &sysInfo );
    MEMORY_BASIC_INFORMATION memInfo;
    LPVOID addr = 0;

    while ( addr < sysInfo.lpMaximumApplicationAddress )
    {
        if ( VirtualQueryEx( process, addr, &memInfo, sizeof( memInfo ) ) == 0 )
        {
            addr = static_cast< char * >( addr ) + sysInfo.dwPageSize;
            continue;
        }

        if ( memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS )
        {
            std::vector<char> buffer( memInfo.RegionSize );
            SIZE_T bytesRead;
            if ( ReadProcessMemory( process, addr, buffer.data( ), buffer.size( ), &bytesRead ) )
            {
                std::string bufferString( buffer.begin( ), buffer.begin( ) + bytesRead );
                std::istringstream iss( bufferString );
                std::string line;

                while ( std::getline( iss, line ) )
                {
                    dump << "0x" << std::setbase( 16 ) << std::setw( 8 ) << std::setfill( L'0' ) << static_cast< long long >( reinterpret_cast < uintptr_t > ( addr ) )
                        << " (" << static_cast< int > ( line.length( ) ) << "): " << line.c_str( ) << "\n";
                }
            }
        }
        addr = static_cast< char * >( addr ) + memInfo.RegionSize;
    }
    dump.close( );
    CloseHandle( process );
}

int main( )
{
    DWORD pid;
    std::wcout << L"Enter the PID: ";
    std::wcin >> pid;
    dump_process_memory( pid, L"dumper_logs.txt" );
    return 0;
}
