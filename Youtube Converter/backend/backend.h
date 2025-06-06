#pragma once
#include <cpr/cpr.h>
#include <sstream>
#include "../entry/main.h"
#include <bit7z/bit7z.hpp>

namespace backend
{
	namespace archieve
	{
		// output path being empty means in the same directory as path
		std::string ExtractArchieve(std::string path, std::string outputPath = "");
	}

	namespace internet
	{
		struct CGetResult
		{
			std::string text;
			std::string url;
			bool success;
			UINT statusCode;
		};

		CGetResult GetRequest(std::string url);
		CGetResult PostRequest(std::string url, cpr::Payload payload = {}, cpr::Header headers = {});
		std::string DownloadFile(std::string url, std::string outputPath);
	}
	namespace console
	{
		static bool showConsole = true;
		static bool showedConsole = false;
		static bool hidedConsole = true;
		static std::atomic<bool> consoleCooldown = false;

		void ToggleConsole(bool show);
	}

    template <typename... T>
    bool RunCmd(T... args)
    {
        std::stringstream cmd;
        (cmd << ... << args);

        if (GetConsoleWindow() != nullptr)
        {
            return (bool)system(cmd.str().c_str());
        }
        else
        {
            HANDLE hNull = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hNull == INVALID_HANDLE_VALUE)
                return false;

            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdOutput = hNull;
            si.hStdError = hNull;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

            BOOL procMake = CreateProcessA(nullptr, (LPSTR)cmd.str().c_str(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
            CloseHandle(hNull);

            if (!procMake)
                return false;

            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode != 0;
        }
    }

	bool DownloadVideo(std::string youtubeUrl, bool mp4);
}