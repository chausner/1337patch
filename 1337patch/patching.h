struct module_info
{
    std::string module_name;
    uint64_t base_address;
};

struct patch_info
{
    std::string module_name;
    uint64_t rva;
    unsigned char original_byte;
    unsigned char patched_byte;
};

uint64_t parse_hex(const std::string &str)
{
    char *end;
    uint64_t result = std::strtoull(str.c_str(), &end, 16);

    if (*end != '\0')
        throw std::runtime_error(("Not a hex number: \"" + str + "\"").c_str());

    return result;
}

std::vector<patch_info> read_1337_file(const std::string &filename)
{
    std::ifstream stream(filename);

    if (stream.bad())
        throw std::runtime_error(("Error opening file \"" + filename + "\".").c_str());

    std::vector<patch_info> patch_infos;

    std::string current_module;

    while (!stream.eof())
    {
        std::string line;

        std::getline(stream, line);

        if (line.find('>', 0) == 0)
        {
            current_module = line.substr(1);
        }
        else
        {
            if (current_module.empty())
                throw std::runtime_error("Invalid 1337 file. Expected module name.");

            size_t colon_pos = line.find(':', 0);
            size_t arrow_pos = line.find("->", 0);

            if (colon_pos == -1 || arrow_pos == -1 || arrow_pos <= colon_pos)
                throw std::runtime_error("Invalid 1337 file.");

            uint64_t rva = parse_hex(line.substr(0, colon_pos));
            uint64_t original_byte = parse_hex(line.substr(colon_pos + 1, arrow_pos - colon_pos - 1));
            uint64_t patched_byte = parse_hex(line.substr(arrow_pos + 2));

            if (original_byte > 255 || patched_byte > 255)
                throw std::runtime_error("Invalid 1337 file.");

            patch_infos.push_back({
                current_module,
                rva,
                static_cast<unsigned char>(original_byte),
                static_cast<unsigned char>(patched_byte) });
        }
    }

    return patch_infos;
}

std::vector<module_info> get_module_infos(HANDLE process)
{
    DWORD bytes_needed;

    if (!EnumProcessModules(process, nullptr, 0, &bytes_needed))
        throw winapi_exception("EnumProcessModules", GetLastError());

    DWORD num_modules = bytes_needed / sizeof(HMODULE);

    std::vector<HMODULE> modules;

    modules.resize(num_modules);

    if (!EnumProcessModules(process, &modules[0], num_modules * sizeof(HMODULE), &bytes_needed))
        throw winapi_exception("EnumProcessModules", GetLastError());

    num_modules = min(num_modules, bytes_needed / sizeof(HMODULE));

    std::vector<module_info> module_infos;

    module_infos.reserve(num_modules);

    for (int i = 0; i < num_modules; i++)
    {
        char buffer[MAX_PATH];

        DWORD length = GetModuleBaseName(process, modules[i], buffer, sizeof(buffer) / sizeof(char));

        if (length == 0)
            throw winapi_exception("GetModuleBaseName", GetLastError());

        std::string moduleName = std::string(buffer, length);

        MODULEINFO module_info;

        if (!GetModuleInformation(process, modules[i], &module_info, sizeof(module_info)))
            throw winapi_exception("GetModuleInformation", GetLastError());

        module_infos.push_back({ moduleName, reinterpret_cast<uint64_t>(module_info.lpBaseOfDll) });
    }

    return module_infos;
}

unsigned char read_byte(HANDLE process, uint64_t address)
{
    unsigned char buffer;
    SIZE_T bytes_read;

    if (!ReadProcessMemory(process, reinterpret_cast<void*>(address), &buffer, sizeof(buffer), &bytes_read) || bytes_read != sizeof(buffer))
        throw winapi_exception("ReadProcessMemory", GetLastError());

    return buffer;
}

void write_byte(HANDLE process, uint64_t address, unsigned char value)
{
    SIZE_T bytes_written;

    if (!WriteProcessMemory(process, reinterpret_cast<void*>(address), &value, sizeof(value), &bytes_written) || bytes_written != sizeof(value))
        throw winapi_exception("WriteProcessMemory", GetLastError());
}

bool verify_patches(HANDLE process, const std::vector<module_info> &module_infos, const std::vector<patch_info> &patch_infos, bool revert)
{
    for (auto&& module_info : module_infos)
    {
        for (auto&& patch_info : patch_infos)
        {
            if (strcmpi(patch_info.module_name.c_str(), module_info.module_name.c_str()) != 0)
                continue;

            unsigned char value = read_byte(process, module_info.base_address + patch_info.rva);

            if (value != (revert ? patch_info.patched_byte : patch_info.original_byte))
                return false;
        }
    }

    return true;
}

void apply_patches(HANDLE process, const std::vector<module_info> &module_infos, const std::vector<patch_info> &patch_infos, bool revert)
{
    for (auto&& module_info : module_infos)
    {
        for (auto&& patch_info : patch_infos)
        {
            if (strcmpi(patch_info.module_name.c_str(), module_info.module_name.c_str()) != 0)
                continue;

            unsigned char value = revert ? patch_info.original_byte : patch_info.patched_byte;

            write_byte(process, module_info.base_address + patch_info.rva, value);
        }
    }
}

DWORD find_process_by_name(const std::string &process_name)
{
    winapi_handle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == 0)
        throw winapi_exception("CreateToolhelp32Snapshot", GetLastError());

    PROCESSENTRY32 process_entry;

    process_entry.dwSize = sizeof(process_entry);

    if (!Process32First(snapshot, &process_entry))
        throw winapi_exception("Process32First", GetLastError());

    do
    {
        if (strcmp(process_entry.szExeFile, process_name.c_str()) == 0)
            return process_entry.th32ProcessID;
    } while (Process32Next(snapshot, &process_entry));

    return -1;
}