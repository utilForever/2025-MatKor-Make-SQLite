#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<std::string> split(std::string s, char c)
{
    std::vector<std::string> res;
    std::string temp;

    for (char x : s)
    {
        if (x == c)
        {
            res.push_back(temp);
            temp = "";
        }
        else
        {
            temp.push_back(x);
        }
    }

    res.push_back(temp);
    return res;
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

unsigned short get_num_tables(std::ifstream& database_file)
{
    char buffer[2];

    database_file.seekg(103);
    database_file.read(buffer, 2);

    return (static_cast<unsigned char>(buffer[1]) |
            static_cast<unsigned char>(buffer[0]) << 8);
}

unsigned short get_page_size(std::ifstream& database_file)
{
    database_file.seekg(16);

    char buffer[2];
    database_file.read(buffer, 2);

    unsigned short page_size = (static_cast<unsigned char>(buffer[1]) |
                                (static_cast<unsigned char>(buffer[0]) << 8));
    return page_size;
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
            unsigned short cell_offset =
                (static_cast<unsigned char>(buffer[1]) |
                 static_cast<unsigned char>(buffer[0]) << 8);

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
    else
    {
        std::vector<std::string> split_command = split(command, ' ');
        std::string target_table_name = split_command.back();

        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file)
        {
            std::cerr << "Failed to open the database file" << std::endl;
            return 1;
        }

        unsigned short page_size = get_page_size(database_file);
        unsigned short num_tables = get_num_tables(database_file);

        char buffer[512];
        int target_root_page = -1;
        std::string create_statement;

        for (int i = 0; i < num_tables; i++)
        {
            database_file.seekg(108 + 2 * i);
            database_file.read(buffer, 2);

            unsigned short cell_offset =
                (static_cast<unsigned char>(buffer[1]) |
                 (static_cast<unsigned char>(buffer[0]) << 8));
            database_file.seekg(cell_offset);

            uint64_t record_size = parse_varint(database_file);
            uint64_t row_id = parse_varint(database_file);
            uint64_t record_header_size = parse_varint(database_file);
            uint64_t type_len = (parse_varint(database_file) - 13) / 2;
            uint64_t name_len = (parse_varint(database_file) - 13) / 2;
            uint64_t tbl_name_len = (parse_varint(database_file) - 13) / 2;
            database_file.ignore(1);
            uint64_t sql_length = (parse_varint(database_file) - 13) / 2;
            database_file.ignore(type_len);
            database_file.ignore(name_len);

            std::string tbl_name(tbl_name_len, ' ');
            database_file.read(tbl_name.data(), tbl_name_len);

            if (tbl_name == target_table_name)
            {
                database_file.read(buffer, 1);
                target_root_page = static_cast<unsigned char>(buffer[0]);

                create_statement.resize(sql_length, '_');
                database_file.read(create_statement.data(), sql_length);

                break;
            }
        }

        if (target_root_page == -1)
        {
            std::cerr << "Failed to find table with name: " << target_table_name
                      << std::endl;
            return 1;
        }

        uint64_t base = (target_root_page - 1) * page_size;

        database_file.seekg(base + 3);
        database_file.read(buffer, 2);

        uint64_t count = (static_cast<unsigned char>(buffer[1]) |
                          (static_cast<unsigned char>(buffer[0]) << 8));

        if (split_command[1] == "count(*)" || split_command[1] == "COUNT(*)")
        {
            std::cout << count << std::endl;
        }
        else
        {
            std::string target_column_name = split_command[1];
            std::string table_name = split_command[3];

            create_statement.pop_back();

            std::vector<std::string> columns =
                split(create_statement.substr(15 + table_name.size()), ',');
            int column_idx = -1;

            for (int i = 0; i < columns.size(); ++i)
            {
                std::vector<std::string> split_column = split(columns[i], ' ');

                for (std::string& s : split_column)
                {
                    std::string t = s;
                    std::erase_if(t, [](unsigned char ch) {
                        return ch == '\n' || ch == '\t';
                    });

                    if (!t.empty())
                    {
                        if (t == target_column_name)
                        {
                            column_idx = i;
                        }

                        break;
                    }
                }

                if (column_idx != -1)
                {
                    break;
                }
            }

            if (column_idx == -1)
            {
                std::cerr << "Failed to find column with name: "
                          << target_column_name << std::endl;
                return 1;
            }

            for (int i = 0; i < count; ++i)
            {
                database_file.seekg(base + 8 + 2 * i);
                database_file.read(buffer, 2);

                unsigned short cell_offset =
                    (static_cast<unsigned char>(buffer[1]) |
                     (static_cast<unsigned char>(buffer[0]) << 8));

                database_file.seekg(base + cell_offset);

                uint64_t record_size = parse_varint(database_file);
                uint64_t row_id = parse_varint(database_file);
                uint64_t record_header_size = parse_varint(database_file);

                std::vector<uint64_t> headers;

                for (int j = 0; j < columns.size(); ++j)
                {
                    headers.push_back(parse_varint(database_file));
                }

                std::vector<std::string> data;

                for (int j = 0; j < columns.size(); ++j)
                {
                    std::string temp;

                    if (headers[j] == 0)
                    {
                        // Do nothing
                    }
                    else if (headers[j] <= 7)
                    {
                        uint64_t size = headers[j];

                        if (headers[j] == 5)
                        {
                            size = 6;
                        }
                        else if (headers[j] == 6)
                        {
                            size = 8;
                        }

                        temp.resize(size, '_');
                        database_file.read(temp.data(), size);
                    }
                    else if (headers[j] >= 12 && headers[j] % 2 == 0)
                    {
                        uint64_t size = (headers[j] - 12) / 2;
                        temp.resize(size, '_');
                        database_file.read(temp.data(), size);
                    }
                    else if (headers[j] >= 13 && headers[j] % 2 == 1)
                    {
                        uint64_t size = (headers[j] - 13) / 2;
                        temp.resize(size, '_');
                        database_file.read(temp.data(), size);
                    }
                    else
                    {
                        std::cerr << "Error parsing data" << std::endl;
                        return 1;
                    }

                    data.push_back(temp);
                }

                std::cout << data[column_idx] << std::endl;
            }
        }
    }

    return 0;
}
