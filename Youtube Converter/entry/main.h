#pragma once
#include <iostream>
#include <filesystem>
#include <future>

#define PATH std::filesystem::current_path().string()
#define YTDLP (PATH + "\\yt-dlp.exe")
#define FFMPEGEXTLESS (PATH + "\\ffmpeg")

#include <backend/backend.h>
#include <frontend/render.h>

inline bool isApiInstalled = false;	
inline bool ytdlpInstalled = false;
inline bool ffmpegInstalled = false;