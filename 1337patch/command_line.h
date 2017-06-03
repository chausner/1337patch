struct command_line
{
    std::string patch_file;
    std::string process_name;
    unsigned int process_id;
    bool revert;
    bool force;

    bool parse(int argc, char *argv[])
    {
        patch_file = std::string();
        process_name = std::string();
        process_id = 0;
        revert = false;
        force = false;

        if (argc < 3)
            return false;

        for (int i = 1; i < argc; i++)
        {
            if (i == 1)
                patch_file = std::string(argv[i]);
            else if (i == 2)
            {
                if (strcmpi(argv[i], "-pid") != 0 && strcmpi(argv[i], "-p") != 0)
                {
                    process_name = std::string(argv[i]);
                }
                else
                {
                    i++;
                    if (i >= argc)
                        return false;
                    process_id = atoi(argv[i]);
                }
            }
            else
            {
                if (strcmpi(argv[i], "-revert") == 0 || strcmpi(argv[i], "-r") == 0)
                    revert = true;
                else if (strcmpi(argv[i], "-force") == 0 || strcmpi(argv[i], "-f") == 0)
                    force = true;
                else
                    return false;
            }
        }

        return true;
    }
};

void print_usage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  1337patch <patch file> <process name> [options]" << std::endl;
    std::cout << "  1337patch <patch file> -pid <pid> [options]" << std::endl << std::endl;
    std::cout << "  patch file:   path to a x64dbg .1337 patch file" << std::endl;
    std::cout << "  process name: image name of the process to patch" << std::endl;
    std::cout << "  pid:          process ID of the process to patch" << std::endl << std::endl;
    std::cout << "Allowed options:" << std::endl;
    std::cout << "  -revert: revert previously applied patches" << std::endl;
    std::cout << "  -force:  apply/revert patches even if patch locations contain unexpected data" << std::endl;
}