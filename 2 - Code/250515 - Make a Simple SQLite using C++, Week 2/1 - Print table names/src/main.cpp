#include <cstring>
#include <fstream>
#include <iostream>

unsigned short get_num_tables(std::ifstream& database_file)
{
    char buffer[2];

    database_file.seekg(103);
    database_file.read(buffer, 2);

    return (static_cast<unsigned char>(buffer[1]) | static_cast<unsigned char>(buffer[0]) << 8);
}

uint64_t parse_varint(std::ifstream& database_file)
{
    uint64_t ret = 0;
    char byte = database_file.get();
    int idx = 0;

    while (byte & 0b10000000 && idx < 8)
    {
        ret = (ret << 7) + (byte & 0b01111111);
        byte = database_file.get();
        idx++;
    }

    if (idx <= 8)
    {
        ret = (ret << 7) + (byte & 0b01111111);
    }
    else
    {
        ret = (ret << 8) + byte;
    }

    return ret;
}

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

        unsigned short num_tables = get_num_tables(database_file);
        std::cout << "number of tables: " << num_tables << std::endl;
    }
    else if (command == ".tables")
    {
        std::ifstream database_file(database_file_path, std::ios::binary);

        if (!database_file)
        {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        unsigned short num_tables = get_num_tables(database_file);

        // NOTE: File header: 100 bytes, Page header: 8 bytes
        char buffer[512];

        for (int i = 0; i < num_tables; ++i)
        {
            database_file.seekg(108 + 2 * i);
            database_file.read(buffer, 2);
            unsigned short cell_offset = (static_cast<unsigned char>(buffer[1]) | static_cast<unsigned char>(buffer[0]) << 8);

            database_file.seekg(cell_offset);

            uint64_t record_size = parse_varint(database_file);
            uint64_t row_id = parse_varint(database_file);
            uint64_t record_header_size = parse_varint(database_file);
            uint64_t type_len = (parse_varint(database_file) - 13) / 2;
            uint64_t name_len = (parse_varint(database_file) - 13) / 2;
            uint64_t tbl_name_len = (parse_varint(database_file) - 13) / 2;
            uint64_t root_page = parse_varint(database_file);
            uint64_t sql = parse_varint(database_file);

            database_file.ignore(type_len);
            database_file.ignore(name_len);

            std::string tbl_name(tbl_name_len, ' ');
            database_file.read(tbl_name.data(), tbl_name_len);

            if (tbl_name == "sqlite_sequence")
            {
                continue;
            }

            std::cout << tbl_name << ' ';
        }

        std::cout << std::endl;
    }

    return 0;
}
