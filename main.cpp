#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include "pass_two.h"
#include "algorithm"
#include "climits"

struct key_val_pair
{
    std::string name;
    int bin_code;

    key_val_pair(const std::string& n, int code) : name(n), bin_code(code) {}
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

    explicit map(int size)
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

    bool find(const std::string& op)
    {
        std::hash<std::string> my_hasher;

        size_t index = my_hasher(op) % hash_table.size();

        // try using std::any_of() for the below checking later when done with the project
        for (const key_val_pair& check : hash_table[index])
        {
            if (check.name == op)
                return true;
        }

        return false;
    }

    int get_code(std::string op)
    {
        std::hash<std::string> my_hasher;

        size_t index = my_hasher(op) % hash_table.size();

        // try using std::any_of() for the below checking later when done with the project
        for (const key_val_pair& check : hash_table[index])
        {
            if (check.name == op)
                return check.bin_code;
        }

        return INT_MIN;
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
    op_tab.insert({"STL", 0x0});
    op_tab.insert({"JSUB", 0x1});
    op_tab.insert({"LDA", 0x2});
    op_tab.insert({"COMP", 0x3});
    op_tab.insert({"JEQ", 0x4});
    op_tab.insert({"J", 0x5});
    op_tab.insert({"STA", 0x6});
    op_tab.insert({"LDL", 0x7});
    op_tab.insert({"RSUB", 0x8});
    op_tab.insert({"END", 0x9});
    op_tab.insert({"JSUB", 0xa});
}

std::string prog_name{};

int main()
{
    /*
     * input file -> populated with the SIC ALP
     * output file -> (intermediate file) going to be populated with the 1st pass of the Two-Pass assembler
     *
    */
    std::ifstream input_file("my.txt");
    std::fstream intermediate_file("outda.txt", std::ios::out); // change to "intermediate.txt"

    if (!input_file.is_open())
    {
        std::cerr << "error opening the Input File\n";
        std::exit(0);
    }
    if(!intermediate_file.is_open())
    {
        std::cerr << "error opening the Output File\n";
        std::exit(0);
    }

    /*
     * op_tab -> static DS consisting of instructions in the SIC, along with their opcode for now. I plan to include the
     *           instruction size in this field too, to make this upward compatible for SIC/XE.
     *
     * sym_tab -> dynamic DS that will hold the pair (LABEL, ADDR_OF_LABEL). Future plans include to add in a new field
     *            to account for DEFINED, MULTI_DEFINED, UNDEFINED to help for better debugging while producing the
     *            listing file.
     *
     * LOC_CTR -> keeps track of the next instruction's location, WHERE the instruction encountered HAS to be written.
     *
     * START_ADDR -> stores where to begin assigning the instructions from, the START address of the program that can
     *               help the loader.
     */
    map op_tab{};
    fill_op_tab(op_tab); // fills the static DS

    map sym_tab{};
    size_t LOC_CTR = 0;
    size_t START_ADDR = 0;

    /*
     * line -> a std::istringstream object, through which I will be extracting the LABEL, OPCODE and OPERAND fields
     *         appropriately.
     *
     * label, opcode and operand are going to store the individuals fields extracted through the istringstream object.
     *
     * add_to_loc -> WHERE should the next instruction begin from, stores that information after each iteration.
     */
    std::string line;
    std::string label, opcode, operand;
    size_t add_to_loc = 0;


    // intermediate_file << "LOC_CTR\tLABEL\tOPCODE\tOPERAND\n\n";
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
                intermediate_file << "ERROR, invalid opcode: " << opcode << " found\n";

                add_to_loc = 0;
                continue;
            }

            bool dup_label = sym_tab.find(label);

            if (!dup_label)
                sym_tab.insert(key_val_pair(label, LOC_CTR));
            else
            {
                std::cout << "ERROR, duplicate label: " << label << " found\n";
                intermediate_file << "ERROR, duplicate label: " << label << " found\n";
                add_to_loc = 0;
                continue;
            }

            std::cout << std::hex << LOC_CTR << '\t' << label << '\t' << opcode
                      << '\t' << operand << std::endl;
            intermediate_file << std::hex << LOC_CTR << '\t' << label << '\t' << opcode
                        << '\t' << operand << std::endl;
        }
        else
        {
            if (label == "END")
            {
                add_to_loc = 1;
                std::cout << std::hex << LOC_CTR << '\t' << '\t' << label << '\t'
                          << '\t' << std::endl;
                intermediate_file << std::hex << LOC_CTR << '\t' << '\t' << label << '\t'
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
                    intermediate_file << "ERROR, invalid opcode: " << opcode << " found\n";

                    add_to_loc = 0;
                    continue;
                }

                std::cout << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                          << operand << std::endl;
                intermediate_file << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                            << operand << std::endl;
            }
        }

        LOC_CTR += add_to_loc;
    }
    
    /* check below later, as I needed to do this ONLY because I added 1 to the "END" label occurrence. I think we can
     * safely neglect that?
     * 
     * closing the input file, as it is not required for the 2nd pass.
    */
    LOC_CTR -= 1;
    input_file.close();
    
    sym_tab.print();
    std::cout << "\n\n";
    /* close the intermediate file to which we were writing to, and then re-open it in read mode.
     * 
     * open a new file to store the object program.
    */
    
    intermediate_file.close();
    intermediate_file.open("outda.txt", std::ios::in);
    std::ofstream obj_file("obj_file.txt");

    if(!intermediate_file.is_open())
    {
        std::cerr << "Could not open the Intermediate File to read from in the 2nd pass\n";
        exit(0);
    }

    if (!obj_file.is_open())
    {
        std::cerr << "Could not create the Object File\n";
        exit(0);
    }
    
    /*
     * as we need 6 bytes for writing the PROG_NAME in the header record, I check below and append x's if necessary
     *
     * (further, need to handle the case where the prog name is just way bigger than 6 before itself)
     */
    while(prog_name.length() < 6)
        prog_name += 'x';

    /*
     * initialise an ostringstream object, which will combine strings together to be further appended as a header record
     *
     *
     */

    /* updating to get the hex value, instead of the decimal equivalent, for the START_ADDR and LENGTH_OF_PROG
     *
     * this is to again, fill in exactly 6 bytes for the start address.
    */

    std::ostringstream oss;
    oss << std::hex << START_ADDR;

    std::string start_add_as_str{};
    start_add_as_str = oss.str();

    while (start_add_as_str.length() < 6)
        start_add_as_str = '0' + start_add_as_str;

    oss.str("");
    oss.clear();

    // again, fill in exactly 6 bytes for the length of the program.
    std::string length_as_str{};
    oss << std::hex << (LOC_CTR - START_ADDR);
    length_as_str = oss.str();

    while (length_as_str.length() < 6)
        length_as_str = '0' + length_as_str;

    oss.str("");
    oss.clear();


    // write the header record to the obj_file
    std::cout << "H_" + prog_name + '_' << start_add_as_str << '_' << length_as_str << std::endl;
    obj_file << "H_" + prog_name + '_' << start_add_as_str << '_' << length_as_str << std::endl;


    /* cooking the text records (currently this is only done for the WORD and BYTE opcodes)
     *
     * UPDATE:no, it doesnt work for JUST the above, i'm expanding to fit in the whole thing itself now.
    */

    std::string loc;
    std::string put_next{};
    std::string text_record{};
    text_record.append("T_");


    bool found_first = false;
    size_t space_taken = 0;
    line.clear();
    std::getline(intermediate_file, line);

    bool loop_check;
    while(std::getline(intermediate_file, line))
    {
        loop_check = false;
        std::istringstream iss(line);

        if(iss >> loc >> label >> opcode >> operand)
        {
            // std::cout << "f\n";
            if(op_tab.find(opcode))
            {
                // get the length of the instruction (have not added that field in the op_tab, but will have to do it
                // for SIC/XE

                int OPC_CODE = op_tab.get_code(opcode);
                int LABEL_ADDR = sym_tab.get_code(operand);

                if(OPC_CODE == INT_MIN)
                {
                    std::cout << "couldn't find the OPC_CODE for" + opcode;
                    exit(0);
                }

                if(LABEL_ADDR == INT_MIN)
                {
                    std::cout << "couldn't find the LABEL_ADDR for" + operand;
                    exit(0);
                }


                // convert OPC_CODE that we got to a string, first converting to hex
                std::string opcode_string{};
                std::ostringstream okk;
                okk << std::hex << OPC_CODE;
                opcode_string = okk.str();

                okk.str("");
                okk.clear();

                // convert the LABEL_ADDR that we got to a string, first converting to hex
                std::string label_string{};

                okk << std::hex << LABEL_ADDR; // Convert integer to hex string
                label_string = okk.str();
                okk.str("");
                okk.clear();

//                std::istringstream tt(hexStr); // Create string stream with hex string
//                std::string a;
//
//                tt >> std::hex >> a; // Read from the stringstream as hex and store in 'a'

//
//                std::istringstream tt(std::to_string(LABEL_ADDR));
//                std::string a;
//
//                tt >> std::hex >> a;
//                std::cout << "found " << opcode_string << '\n';

                while(opcode_string.length() < 2)
                    opcode_string = '0' + opcode_string;

                while(label_string.length() < 4)
                    label_string = '0' + label_string;

                // need to handle care of the 'x' bit for indexing mode later

                put_next += '_' + (opcode_string + label_string);
                // std::cout << opcode_string << ' ' << label_string << ' ' << put_next << '\n';
                space_taken += 3;

                if(space_taken > 30)
                {

                }
                else if(space_taken == 30)
                {

                }
                else
                    continue;
            }
            else if (opcode == "BYTE")
            {
                std::string token{};

                // loop_check = true;
                if (operand[0] == 'C')
                {
                    // ex: 42069 ARRAY BYTE C'GITHUB'

                    int i = 2;
                    do
                    {
                        token = token + std::to_string(static_cast<int>(operand[i]));
                        i++;
                    } while (operand[i] != '\'');

                    // using a temp string token for much better readability, to allow checks later when space_taken
                    // checks are made

                    put_next += '_' + token;
                    space_taken += operand.length() - 3; // remove 3 count because 'C' and the single quotes (' ')

                    if(space_taken > 30)
                    {

                    }
                    else if(space_taken == 30)
                    {

                    }
                    else
                        continue;
                }
                else if (operand[0] == 'X')
                {
                    token.clear();
                    /* ex: 42069 VAL BYTE X'1b'
                     *
                     * Currently, this accepts hex consts only as a pair of numbers, i.e, the length is even.
                     * Not a big deal to change it to accept odd lengths too, so will change it later.
                    */

                    // n -> NUMBER of chars to be collected, NOT the last index to be retreived
                    token += operand.substr(2, operand.length() - 3);

                    while(token.length() < 6)
                        token = '0' + token;

                    space_taken += (operand.length() - 3) / 2;
                    put_next += '_' + token;

                    if(space_taken > 30)
                    {

                    }
                    else if(space_taken == 30)
                    {

                    }
                    else
                        continue;
                }
            } // end opcode == "BYTE"
            else if (opcode == "WORD")
            {
                std::string token{};
                // ex: 1000 COUNTER WORD 256
                //loop_check = true;
                std::istringstream in(operand);
                int val;
                in >> val;

                std::ostringstream ou;
                ou << std::hex << val;
                token = ou.str();

                ou.str("");
                ou.clear();

                while(token.length() < 6)
                    token += '0' + token;

                put_next += '_' + token;
                space_taken += 3;

                if(space_taken > 30)
                {

                }
                else if(space_taken == 30)
                {

                }
                else
                    continue;

            } // end opcode == "WORD"

            // i think not needed below because we are adding at each iteration itself
            // put_next += '_';

            if(!found_first)
            {
                loc = "00" + loc;
                text_record.append(loc + '_');
                // obj_file << loc << '_' << 30 << '_';
                found_first = true;
            }

            if(space_taken == 30)
            {
                text_record.append(std::to_string(space_taken) + '_');
                text_record.append(put_next);
                obj_file << text_record << std::endl;

                put_next.clear();
                space_taken = 0;
                found_first = false;
            }
        }
        else if(operand.empty() && opcode.empty())
        {

        }
        else if(operand.empty())
        {
            //                loc   label   opcode
            // something like 1000   LDA    LENGTH (without any label)

            operand = opcode;
            opcode = label;

            if(op_tab.find(opcode))
            {
                if(sym_tab.find(operand))
                {

                }
            }

        }


//        // obj_file << put_next << '_';
//        text_record += put_next;
//        put_next.clear();
    }

    obj_file.close();

    /*

    intermediate_file << "\n\nLength of Program is " << std::dec
                << (LOC_CTR - START_ADDR) << " bytes in decimal system\n";
    std::cout << "\n\nLength of Program is " << std::dec << LOC_CTR - START_ADDR
              << " bytes in decimal system\n";

    intermediate_file << "\n\nLength of Program is " << std::hex << LOC_CTR - START_ADDR
                << " bytes in hex system\n";
    std::cout << "\n\nLength of Program is " << std::hex << LOC_CTR - START_ADDR
              << " bytes in hex system\n";

    std::cout << "\nSYMBOL TAB\n\n";
    sym_tab.print();

    std::cout << "\nOPCODE TAB\n\n";
    op_tab.print();

    */
    intermediate_file.close();

    return 0;
}