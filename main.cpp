#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include "pass_two.h"

struct key_val_pair
{
    std::string name;
    int bin_code;

    key_val_pair(const std::string& n, int code) : name(n), bin_code(code)
    {
    }
};

class map
{
private:
    std::vector<std::vector<key_val_pair>> hash_table;

public:
    map()
    {
        hash_table.resize(101);
    }

    map(int size)
    {
        hash_table.resize(size);
    }

    void insert(key_val_pair i)
    {
        std::string str = i.name;
        std::hash<std::string> my_hasher;

        size_t index =
                my_hasher(str) % hash_table.size();  // check if we need to subtract 1

        hash_table[index].push_back(i);
    }

    bool find(std::string op)
    {
        std::hash<std::string> my_hasher;

        size_t index = my_hasher(op) % hash_table.size();

        for (const key_val_pair& check : hash_table[index])
        {
            if (check.name == op)
                return true;
        }

        return false;
    }

    void print()
    {
        for (size_t i = 0; i < hash_table.size(); ++i)
        {
            for (const key_val_pair& pair : hash_table[i])
                std::cout << "(" << pair.name << ", " << pair.bin_code << ")\n";
        }
    }
};

void fill_op_tab(map& op_tab)
{
    op_tab.insert({"STL", 0});
    op_tab.insert({"JSUB", 1});
    op_tab.insert({"LDA", 2});
    op_tab.insert({"COMP", 3});
    op_tab.insert({"JEQ", 4});
    op_tab.insert({"J", 5});
    op_tab.insert({"STA", 6});
    op_tab.insert({"LDL", 7});
    op_tab.insert({"RSUB", 8});
    op_tab.insert({"END", 9});
}

std::string prog_name{};

int main()
{
    std::ifstream input_file("my.txt");
    std::ofstream output_file("outda.txt");

    map op_tab{};
    map sym_tab{};
    size_t LOC_CTR = 0;
    size_t START_ADDR = 0;

    fill_op_tab(op_tab);

    if (!input_file.is_open() || !output_file.is_open())
    {
        std::cerr << "error opening the file\n";
        std::exit(0);
    }

    std::string line;
    std::string label, opcode, operand;
    size_t add_to_loc = 0;

    // output_file << "LOC_CTR\tLABEL\tOPCODE\tOPERAND\n\n";
    std::cout << "LOC_CTR\tLABEL\tOPCODE\tOPERAND\n\n";

    while (std::getline(input_file, line))
    {
        std::istringstream iss(line);

        iss >> label;

        if (iss >> opcode >> operand)
        {
            bool valid_opcode = op_tab.find(opcode);
            if (valid_opcode)
                add_to_loc = 3;

            else if (opcode == "START")
            {
                prog_name = label;
                std::istringstream hex(operand);
                hex >> std::hex >> LOC_CTR;
                START_ADDR = LOC_CTR;
            }
            else if (opcode == "WORD")
            {
                add_to_loc = 3;
            }
            else if (opcode == "BYTE")
            {
                if (operand[0] == 'C')
                {
                    add_to_loc = operand.length() - 3;
                }
                else if (operand[0] == 'X')
                {
                    add_to_loc = (operand.length() - 3) / 2;
                }
            }
            else if (opcode == "RESW")
            {
                std::istringstream to_skip(operand);
                size_t temp;

                to_skip >> temp;
                add_to_loc = temp * 3;
            }
            else if (opcode == "RESB")
            {
                std::istringstream to_skip(operand);
                size_t temp;

                to_skip >> temp;
                add_to_loc = temp;
            }
            else
            {
                std::cout << "ERROR, invalid opcode: " << opcode << " found\n";
                output_file << "ERROR, invalid opcode: " << opcode << " found\n";

                add_to_loc = 0;
                continue;
            }

            bool dup_label = sym_tab.find(label);

            if (!dup_label)
                sym_tab.insert(key_val_pair(label, LOC_CTR));
            else
            {
                std::cout << "ERROR, duplicate label: " << label << " found\n";
                output_file << "ERROR, duplicate label: " << label << " found\n";
                add_to_loc = 0;
                continue;
            }

            std::cout << std::hex << LOC_CTR << '\t' << label << '\t' << opcode
                      << '\t' << operand << std::endl;
            output_file << std::hex << LOC_CTR << '\t' << label << '\t' << opcode
                        << '\t' << operand << std::endl;
        }
        else
        {
            if (label == "END")
            {
                add_to_loc = 1;
                std::cout << std::hex << LOC_CTR << '\t' << '\t' << label << '\t'
                          << '\t' << std::endl;
                output_file << std::hex << LOC_CTR << '\t' << '\t' << label << '\t'
                            << '\t' << std::endl;
            }
            else
            {
                operand = opcode;
                opcode = label;

                bool valid_op_code = op_tab.find(opcode);
                if (valid_op_code)
                    add_to_loc = 3;
                else
                {
                    std::cout << "ERROR, invalid opcode: " << opcode << " found\n";
                    output_file << "ERROR, invalid opcode: " << opcode << " found\n";

                    add_to_loc = 0;
                    continue;
                }

                std::cout << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                          << operand << std::endl;
                output_file << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                            << operand << std::endl;
            }
        }

        LOC_CTR += add_to_loc;
    }

    LOC_CTR -= 1;

    std::ofstream obj_file("obj_file.txt");

    // appending header record
    if (!obj_file.is_open())
    {
        std::cerr << "could not open object file\n";
        exit(0);
    }

    while (prog_name.length() != 6)
        prog_name += 'x';

    std::ostringstream oss;
    oss << std::hex << START_ADDR;
    std::cout << "printing " + oss.str() << "  " << oss.str().length()
              << std::endl;
    std::string start_add_as_str = std::to_string(START_ADDR);
    // updating to get the hex value, instead of the decimal equivalent,
    start_add_as_str = oss.str();
    oss.clear();

    std::string length_as_str{};  //= std::to_string(LOC_CTR - START_ADDR);
    std::ostringstream okk;
    okk << std::hex << (LOC_CTR - START_ADDR);

    std::cout << "LEN IS " << std::hex << LOC_CTR - START_ADDR << std::endl;
    length_as_str = okk.str();
    std::cout << "printing " + length_as_str << "   " << length_as_str.length()
              << '\n';
    okk.clear();

    while (start_add_as_str.length() != 6)
        start_add_as_str = '0' + start_add_as_str;

    while (length_as_str.length() != 6)
        length_as_str = '0' + length_as_str;

    obj_file << "H_" + prog_name + '_' << start_add_as_str << '_' << length_as_str
             << std::endl;


    output_file.close();

    std::ifstream pass2("out.txt");

    std::string loc;
    std::string put_next{};
    obj_file << "T_";
    bool found_first = false;
    size_t space_taken = 0;
    line.clear();

    bool loop_check = false;

    while (std::getline(pass2, line))
    {
        loop_check = false;
        std::istringstream iss(line);
        if (iss >> loc >> label >> opcode >> operand)
        {
            if (opcode == "BYTE")
            {
                loop_check = true;
                if (operand[0] == 'C')
                {
                    int i = 2;
                    do
                    {
                        put_next = put_next + std::to_string(static_cast<int>(operand[i]));
                        i++;
                    } while (operand[i] != '\'');

                    space_taken += operand.length() - 3;
                }

                else if (operand[0] == 'X')
                {
                    put_next = operand.substr(2, operand.length() - 3);

                    while(put_next.length() != 6)
                        put_next = '0' + put_next;

                    space_taken += (operand.length() - 3) / 2;
                }
            }
            else if (opcode == "WORD")
            {
                loop_check = true;
                put_next = operand;

                while(put_next.length() != 6)
                    put_next = '0' + put_next;

            }

            if (!found_first)
            {
                loc = "00" + loc;
                obj_file << loc << '_' << 30 << '_';
                found_first = true;
            }
            if(loop_check)
                obj_file << put_next << '_';
        }
        put_next.clear();
    }

    obj_file.close();

    /*

    output_file << "\n\nLength of Program is " << std::dec
                << (LOC_CTR - START_ADDR) << " bytes in decimal system\n";
    std::cout << "\n\nLength of Program is " << std::dec << LOC_CTR - START_ADDR
              << " bytes in decimal system\n";

    output_file << "\n\nLength of Program is " << std::hex << LOC_CTR - START_ADDR
                << " bytes in hex system\n";
    std::cout << "\n\nLength of Program is " << std::hex << LOC_CTR - START_ADDR
              << " bytes in hex system\n";

    std::cout << "\nSYMBOL TAB\n\n";
    sym_tab.print();

    std::cout << "\nOPCODE TAB\n\n";
    op_tab.print();

    */
    input_file.close();
    pass2.close();

    return 0;
}