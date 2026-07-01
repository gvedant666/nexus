#pragma once
#include <string>
#include <unistd.h>


// A move-only RAII wrapper that guarantees the lock file is deleted.
class UniqueFileLock
{
public:
    UniqueFileLock() = default;

    // Constructor acquires ownership of the lock path
    UniqueFileLock(std::string phyName, std::string lockPath)
        : phy_name_(std::move(phyName)), lock_path_(std::move(lockPath))
    {
    }

    UniqueFileLock(const UniqueFileLock&) = delete;
    UniqueFileLock& operator=(const UniqueFileLock&) = delete;

    UniqueFileLock(UniqueFileLock&& other) noexcept
        : phy_name_(std::move(other.phy_name_)), lock_path_(std::move(other.lock_path_))
    {
        other.lock_path_.clear();
    }

    UniqueFileLock& operator=(UniqueFileLock&& other) noexcept
    {
        if (this != &other)
        {
            release();
            phy_name_ = std::move(other.phy_name_);
            lock_path_ = std::move(other.lock_path_);
            other.lock_path_.clear();
        }
        return *this;
    }

    ~UniqueFileLock()
    {
        release();
    }

    const std::string& get() const { return phy_name_; }
    bool isValid() const { return !lock_path_.empty(); }

private:
    void release()
    {
        if (!lock_path_.empty())
        {
            unlink(lock_path_.c_str());
            lock_path_.clear();
        }
    }

    std::string phy_name_;
    std::string lock_path_;
};
