class winapi_exception : public std::runtime_error
{
public:
    winapi_exception(const char *function, DWORD error_code) : std::runtime_error(get_message(function, error_code).c_str())
    {
    }

private:
    std::string get_message(const char *function, DWORD error_code)
    {
        std::ostringstream message;

        message << function << " failed with error code 0x";
        message << std::hex << std::setw(8) << std::setfill('0') << error_code;
        message << ": " << get_winapi_error_message(error_code);

        return message.str();
    }

    std::string get_winapi_error_message(DWORD error_code)
    {
        char *buffer;

        DWORD length = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error_code, 0, reinterpret_cast<char*>(&buffer), 0, nullptr);

        if (length == 0)
            return "";

        std::string message(buffer, length);

        LocalFree(buffer);

        return message;
    }
};

class winapi_handle
{
public:
    winapi_handle(HANDLE handle)
    {
        this->handle = handle;
    }

    ~winapi_handle()
    {
        if (handle != 0)
            CloseHandle(handle);
    }

    operator HANDLE() const
    {
        return handle;
    }

private:
    HANDLE handle;
};