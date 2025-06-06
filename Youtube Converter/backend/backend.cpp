#include "backend.h"

backend::internet::CGetResult backend::internet::GetRequest(std::string url)
{
	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		cpr::Header{ {"User-Agent", "Mozilla/5.0"}, {"accept-language", "en-US,en"} }
	);

	CGetResult newResult;
	newResult.text = r.text;
	newResult.success = r.status_code == 200;
	newResult.statusCode = r.status_code;
	newResult.url = url;

	return newResult;
}

backend::internet::CGetResult backend::internet::PostRequest(std::string url, cpr::Payload payload, cpr::Header headers)
{
	cpr::Response r = cpr::Post(url, payload, headers);

	CGetResult newResult;
	newResult.text = r.text;
	newResult.url = url;
	newResult.statusCode = r.status_code;
	newResult.success = r.status_code == 200;

	return newResult;
}

std::string backend::internet::DownloadFile(std::string url, std::string outputPath)
{
	cpr::Response r = cpr::Get(cpr::Url{ url });
	std::cout << "Downloading file " << url << "\n";

	if (r.status_code == 200)
	{
		std::ofstream out(outputPath, std::ios::binary);
		if (out.is_open())
		{
			out.write(r.text.c_str(), r.text.size());
			out.close();
		}
	}
	else
	{
		outputPath = "";
	}

	return outputPath;
}

bool backend::DownloadVideo(std::string youtubeUrl, bool mp4)
{
	return RunCmd("\"" + YTDLP + "\" -f " + (mp4 ? "bestvideo+bestaudio --merge-output-format mp4 " 
												 : "bestaudio --extract-audio --audio-format mp3 ") + youtubeUrl);
}

void backend::console::ToggleConsole(bool show)
{
	if (consoleCooldown)
		return;

	consoleCooldown = true;

	std::thread([show]()
		{
			if (show)
			{
				if (!showedConsole)
				{
					AllocConsole();
					FILE* fp;

					freopen_s(&fp, "CONIN$", "r", stdin);
					freopen_s(&fp, "CONOUT$", "w", stdout);
					freopen_s(&fp, "CONOUT$", "w", stderr);

					ShowWindow(GetConsoleWindow(), SW_SHOW);
					showedConsole = true;
					hidedConsole = false;
				}
			}
			else
			{
				if (!hidedConsole)
				{
					HWND hwnd = GetConsoleWindow();
					ShowWindow(hwnd, SW_HIDE);

					fclose(stdin);
					fclose(stdout);
					fclose(stderr);
					FreeConsole();
					hidedConsole = true;
					showedConsole = false;
				}
			}

			// Prevent toggle spam (cooldown 500ms)
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			consoleCooldown = false;
		}).detach();
}

std::string backend::archieve::ExtractArchieve(std::string path, std::string outputPath)
{
	try {
		using namespace bit7z;


		Bit7zLibrary lib{ "7z.dll" };
		BitArchiveReader archive{ lib, path, BitFormat::SevenZip };

		if (outputPath.empty())
		{
			outputPath = path;
		}
		std::cout << "Extracting " << path;

		archive.extractTo((std::filesystem::path(outputPath).parent_path() / std::filesystem::path(outputPath).stem()).string());
		return "";
	}
	catch (const bit7z::BitException& ex)
	{
		return ex.what();
	}
}
