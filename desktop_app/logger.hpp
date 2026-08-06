#ifndef ROTATINGLOGGER_H
#define ROTATINGLOGGER_H

#include <fstream>
#include <mutex>
#include <string>
#include <cstddef>

class RotatingLogger
{
private:
    std::ofstream m_file;
    std::mutex m_mutex;
    std::string m_basePath;
    const std::size_t m_maxSize = 5 * 1024 * 1024; // 5 MB
    const std::size_t m_maxFiles = 3;              // Keep 3 old logs

    void rotate();

public:
    explicit RotatingLogger(const std::string &path);
    void log(const std::string &msg);
};

#endif // ROTATINGLOGGER_H