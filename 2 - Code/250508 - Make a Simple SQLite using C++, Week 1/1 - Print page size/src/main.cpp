#include <cstring>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3)
    {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    if (command == ".dbinfo")
    {
        std::ifstream database_file(database_file_path, std::ios::binary);

        if (!database_file)
        {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        char buffer[2];

        database_file.seekg(16);
        database_file.read(buffer, 2);
        unsigned short page_size = (buffer[1] | buffer[0] << 8);
        std::cout << "database page size: " << page_size << std::endl;
    }

    return 0;
}
