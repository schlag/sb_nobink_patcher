#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <shlobj.h>
#include "main.h"

int main(int argc, char* argv[])
{
	SetConsoleTitleA("SB NoBink Patcher");
	char cArgBuf[MAX_PATH];
	if (argc > 1)
	{
		strcpy(cArgBuf, argv[1]);
		if (strstr(cArgBuf, ".lnk"))
		{
			char cLinkTarget[MAX_PATH];
			memset(cLinkTarget, 0, MAX_PATH);
			GetShortcutTarget(cArgBuf, cLinkTarget);
			if (strlen(cLinkTarget) > 0)
			{
				StripFilename(cLinkTarget);
				SetCurrentDirectoryA(cLinkTarget);
			}
			
		}
		else
		{
			StripFilename(cArgBuf);
			SetCurrentDirectoryA(cArgBuf);
		}
	}
	long lFileSize = 0;
	char pDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, pDir);
	FILE *pFile = fopen("sb.exe", "r");
	if(!pFile)
	{
		printf("USAGE: Simply place this file inside your Shadowbane directory and then run it.\nYou can also drag and drop sb.exe directly onto this file.\n\nsb.exe was not found in:\n\"%s\"\n", pDir);
	}
	else
	{
		fclose(pFile);
		pFile = fopen("sb.exe" , "r+b");
		if (pFile != NULL)
		{
			fseek(pFile, 0, SEEK_END);
			lFileSize = ftell(pFile);
			rewind(pFile);
			if (lFileSize > 0)
			{
				char *pcFileContents = (char*)malloc((sizeof(char)*lFileSize));
				memset(pcFileContents, 0, (sizeof(char)*lFileSize));
				if (pcFileContents != NULL && fread(pcFileContents, 1, lFileSize, pFile) == lFileSize)
				{
					CCRC CRC;
					CRC.Init_CRC32_Table();
					int FileCRC = CRC.Get_CRC(pcFileContents, lFileSize);
					if (FileCRC == 0x9C94E98A || FileCRC == 0x18488b2c)
					{
						printf("Correct version of sb.exe detected at:\n\"%s\\sb.exe\"\n\nWould you like to apply the NoBink Patch? [y/N]: ", pDir);
						char result = getchar();
						if (result == 'y' || result == 'Y')
						{
							printf("Patching sb.exe...");
							int Payload[5] = { 0x33, 0xC0, 0x5E, 0x5D, 0xC3 };
							fseek(pFile, 0x563557, SEEK_SET);
							for (int i = 0; i < 5; i++)
							{
								fputc(Payload[i], pFile);
							}
							printf("Done!\n");
						}
						else
						{
							printf("Not applying NoBink Patch to sb.exe\n");
						}
					}
					else if (FileCRC == 0x85BC050B || FileCRC == 0x016067ad)
					{
						printf("This sb.exe has already been patched.\nWould you like to remove the NoBink Patch? [y/N]: ");
						char result = getchar();
						if (result == 'y' || result == 'Y')
						{
							printf("Removing the NoBink Patch from sb.exe...");
							int Payload[5] = { 0x6A, 0x00, 0x50, 0xFF, 0x15 };
							fseek(pFile, 0x563557, SEEK_SET);
							for (int i = 0; i < 5; i++)
							{
								fputc(Payload[i], pFile);
							}
							printf("Done!\n");
						}
						else
						{
							printf("Nothing to do.\n");
						}
					}
					else
					{
						printf("Incorrect version of sb.exe detected.\nPlease ensure you have the latest version.\n");
					}
				}
				free(pcFileContents);
			}
			fclose(pFile);
		}
		else
		{
			printf("Failed to open sb.exe. Please ensure sb.exe is not currently running.\n");
		}
	}
	printf("\nPress any key to exit.");
	_getch();
	return 0;
}

void CCRC::Init_CRC32_Table()
{
      unsigned long ulPolynomial = 0x04c11db7;
      for(int i = 0; i <= 0xFF; i++)
      {
            crc32_table[i]=Reflect(i, 8) << 24;
            for (int j = 0; j < 8; j++)
			{
                  crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
			}
            crc32_table[i] = Reflect(crc32_table[i], 32);
      }
}

unsigned long CCRC::Reflect(unsigned long ref, char ch)
{
      unsigned long value = 0;
      for(int i = 1; i < (ch + 1); i++)
      {
            if(ref & 1)
			{
                  value |= 1 << (ch - i);
			}
            ref >>= 1;
      }
      return value;
}

int CCRC::Get_CRC(char* data, int size)
{
      unsigned long ulCRC = 0xffffffff;
      int len = size;
      unsigned char* buffer;
      buffer = (unsigned char*)data;
      while(len--)
	  {
            ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *buffer++];
	  }
      return ulCRC ^ 0xffffffff;
}

HRESULT GetShortcutTarget(const char* lpszLinkFile, char* lpszPath)
{
	HRESULT hres;
	IShellLink* psl;
	wchar_t wszGotPath[MAX_PATH];
	WIN32_FIND_DATA wfd;

	*lpszPath = 0;

	hres = CoInitialize(NULL);

	if(!SUCCEEDED(hres))
	{
	   return 0;
	}
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);
			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(0, 0);
				if (SUCCEEDED(hres))
				{
					hres = psl->GetPath(wszGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
					if (FAILED(hres))
					{
						return 0;
					}
					wcstombs(lpszPath, wszGotPath, MAX_PATH);
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return hres;
}

void StripFilename(char *path)
{
	int len = strlen(path);
	for (int i = len; i >= 0; i--)
	{
		if (path[i] == '\\')
		{
			path[i+1] = 0;
			break;
		}
	}
}