#include <mutex>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include "Log.h"
#include "Path.h"

namespace nilou {

    namespace encode {
        std::string wideToMulti(std::wstring_view sourceStr, UINT pagecode)
        {
            auto newLen = WideCharToMultiByte(pagecode, 0, sourceStr.data(), sourceStr.size(), 
                                            nullptr, 0, nullptr, nullptr);

            std::string targetStr;
            targetStr.resize(newLen);
            WideCharToMultiByte(pagecode, 0, sourceStr.data(), sourceStr.size(), 
                                &targetStr[0], targetStr.size(), nullptr, nullptr);
            return targetStr;
        }

        std::string wideToUtf8(std::wstring_view sourceWStr)
        {
            return wideToMulti(sourceWStr, 65001);
        }
        std::string wideToOme(std::wstring_view sourceWStr)
        {
            return wideToMulti(sourceWStr, CP_OEMCP);
        }
    }

    std::ostream &operator<<(std::ostream &os, const wchar_t *strPt)
    {
        return os << std::wstring_view(strPt);
    }
    std::ostream &operator<<(std::ostream &os, std::wstring_view str)
    {
        return os << encode::wideToOme(str);
    }

    std::ofstream CreateLogFile()
    {
        std::filesystem::path LogsDir = FPath::ProjectDir() / "Saved/Logs";
        std::filesystem::path CurrentPath = LogsDir / "Nilou.log";
        if (std::filesystem::exists(CurrentPath))
        {
            auto LastWriteTime = std::filesystem::last_write_time(CurrentPath);
            auto TimeT = std::chrono::system_clock::to_time_t(
                std::chrono::clock_cast<std::chrono::system_clock>(LastWriteTime));
            std::stringstream ss;
            ss << std::put_time(std::localtime(&TimeT), "Nilou_%Y%m%d_%H%M%S.log");
            std::filesystem::rename(CurrentPath, LogsDir / ss.str());
        }
        std::filesystem::create_directories(LogsDir);
        std::ofstream LogFile{CurrentPath};
        return LogFile;
    }

    void Logf_Internal(ELogVerbosity Verbosity, const std::string& Message)
    {
        static std::mutex Mutex;
        std::lock_guard<std::mutex> Lock(Mutex);
        static std::ofstream File = CreateLogFile();
        // 生成带有毫秒的时间戳字符串
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
    #if defined(_WIN32)
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif
        char timestamp[40];
        std::strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &tm_now);
        std::stringstream ss_timestamp;
        ss_timestamp << timestamp << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        std::string timestamp_with_ms = ss_timestamp.str();

        std::cout << timestamp_with_ms << Message;
        std::cout.flush();
        File << timestamp_with_ms << Message;
        File.flush();
    }
}