#include "main.h"

//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    // Promises and futures for signaling completion
    std::promise<void> ytdlpPromise;
    std::promise<void> ffmpegPromise;

    std::future<void> ytdlpFuture = ytdlpPromise.get_future();
    std::future<void> ffmpegFuture = ffmpegPromise.get_future();

    // YTDLP thread
    std::thread([](std::promise<void> p) {
        std::string ytdlpPath = YTDLP;

        if (!std::filesystem::is_regular_file(ytdlpPath))
        {
            auto res = backend::internet::DownloadFile(
                "https://github.com/yt-dlp/yt-dlp/releases/download/2025.05.22/yt-dlp.exe",
                ytdlpPath
            );
            if (res.empty())
            {
                MessageBoxA(0, "Failed to install yt-dlp API", "Error", MB_ICONERROR | MB_OK);
                p.set_value();
                return;
            }
            ytdlpInstalled = true;
        }
        else
        {
            ytdlpInstalled = true;
        }
        p.set_value();
        }, std::move(ytdlpPromise)).detach();

    // FFMPEG thread
    std::thread([](std::promise<void> p) {
        if (!std::filesystem::is_regular_file(FFMPEGEXTLESS + ".exe"))
        {
            if (!std::filesystem::is_regular_file(FFMPEGEXTLESS + ".7z"))
            {
                std::string res = backend::internet::DownloadFile(
                    "https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-2025-06-04-git-a4c1a5b084-essentials_build.7z",
                    FFMPEGEXTLESS + ".7z"
                );
                if (res.empty())
                {
                    MessageBoxA(0, "Failed to install sound API", "Error", MB_ICONERROR | MB_OK);
                    p.set_value();
                    return;
                }
            }

            auto apiInstalled = backend::archieve::ExtractArchieve(FFMPEGEXTLESS + ".7z");
            if (!apiInstalled.empty())
            {
                MessageBoxA(0, ("Failed to extract sound API. Reason: " + apiInstalled).c_str(), "Error", MB_ICONERROR | MB_OK);
                p.set_value();
                return;
            }

            std::string extractedPath = FFMPEGEXTLESS;

            // Find first subfolder in extracted path
            std::string firstSubfolder;
            {
                WIN32_FIND_DATAA findData;
                HANDLE hFind = FindFirstFileA((extractedPath + "\\*").c_str(), &findData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                            strcmp(findData.cFileName, ".") != 0 &&
                            strcmp(findData.cFileName, "..") != 0)
                        {
                            firstSubfolder = findData.cFileName;
                            break;
                        }
                    } while (FindNextFileA(hFind, &findData));
                    FindClose(hFind);
                }
                if (firstSubfolder.empty())
                {
                    MessageBoxA(0, "Failed to find extracted subfolder", "Error", MB_ICONERROR | MB_OK);
                    p.set_value();
                    return;
                }
            }

            std::string binPath = extractedPath + "\\" + firstSubfolder + "\\bin\\ffmpeg.exe";
            std::string targetPath = (PATH + "\\ffmpeg.exe");

            if (!CopyFileA(binPath.c_str(), targetPath.c_str(), FALSE))
            {
                MessageBoxA(0, "Failed to copy ffmpeg.exe", "Error", MB_ICONERROR | MB_OK);
                p.set_value();
                return;
            }

            DeleteFileA((FFMPEGEXTLESS + ".7z").c_str());

            // Recursive delete function
            std::function<void(const std::string&)> deleteFolder;
            deleteFolder = [&](const std::string& folder)
                {
                    WIN32_FIND_DATAA fd;
                    HANDLE hFind = FindFirstFileA((folder + "\\*").c_str(), &fd);
                    if (hFind != INVALID_HANDLE_VALUE)
                    {
                        do
                        {
                            if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
                                continue;

                            std::string filePath = folder + "\\" + fd.cFileName;

                            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                                deleteFolder(filePath);
                            else
                                DeleteFileA(filePath.c_str());
                        } while (FindNextFileA(hFind, &fd));
                        FindClose(hFind);
                    }
                    RemoveDirectoryA(folder.c_str());
                };

            deleteFolder(extractedPath);

            ffmpegInstalled = true;
        }
        else
        {
            ffmpegInstalled = true;
        }
        p.set_value();
        }, std::move(ffmpegPromise)).detach();

    // Thread waiting for both to finish
    std::thread([](std::future<void> f1, std::future<void> f2)
        {
            f1.wait();
            f2.wait();

            isApiInstalled = (ytdlpInstalled && ffmpegInstalled);
        }, std::move(ytdlpFuture), std::move(ffmpegFuture)).detach();

	Render render;
	if (!render.CreateFrontendWindow("Youtube Man", 800, 500))
		return -1;
	render.FrontendRender();
}
