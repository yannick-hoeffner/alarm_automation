#include "logger.hpp"

#include <chrono>
#include <filesystem>

RotatingLogger::RotatingLogger(const std::string &path) : m_basePath(path)
{
    m_file.open(m_basePath, std::ios::app);
}

void RotatingLogger::log(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check size (approximate)
    if (m_file.tellp() > static_cast<std::streamoff>(m_maxSize))
    {
        rotate();
    }

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    m_file << std::format("[{:%Y-%m-%dT%H:%M:%S}.{:03}] ",
                          std::chrono::floor<std::chrono::seconds>(now),
                          ms.count())
           << msg << "\n";

    m_file.flush();
}

void RotatingLogger::rotate()
{
    m_file.close();

    // Rename existing .log.1 to .log.2, .log to .log.1
    // This is a simple rotation
    for (int i = (int)m_maxFiles - 1; i >= 1; --i)
    {
        std::string oldName = m_basePath + "." + std::to_string(i);
        std::string newName = m_basePath + "." + std::to_string(i + 1);

        std::filesystem::remove(newName); // Delete oldest
        std::filesystem::rename(oldName, newName);
    }

    std::filesystem::rename(m_basePath, m_basePath + ".1");

    m_file.open(m_basePath, std::ios::app);
}