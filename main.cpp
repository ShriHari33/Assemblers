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
    op_tab.insert({"RSUB", 0x4C});
    op_tab.insert({"END", 0x9});
    op_tab.insert({"JSUB", 0xa});
    op_tab.insert({"ADD", 0xb});

}

bool isDirective(std::string str)
{
    if(str == "RESB" || str == "RESW")
        return true;

    return false;
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
        label.clear(), opcode.clear(), operand.clear();
        std::istringstream iss(line);

        iss >> label;

        // if there are  "LABEL, OPCODE, OPERAND", only then we go into this 'if'
        if(iss >> opcode >> operand)
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
        else if(opcode.empty() && operand.empty() && label.empty())
            continue;
        else if(opcode.empty() && operand.empty())
        {
            // only such instruction in SIC is "RSUB"

            opcode = label;

            if(op_tab.find(opcode))
            {
                std::cout << std::hex << LOC_CTR << '\t' << '\t' << opcode << std::endl;
                intermediate_file << std::hex << LOC_CTR << '\t' << '\t' << opcode << std::endl;

                // I only read our file until I find the "END". I don't care what happens next, I just stop.
                if(opcode == "END")
                {
                    add_to_loc = 1;
                    LOC_CTR += 1;
                    break;
                }
                else
                    add_to_loc = 3;
            }
            else
            {
                std::cout << "ERROR, invalid opcode: " << opcode << " found\n";
                intermediate_file << "ERROR, invalid opcode: " << opcode << " found\n";

                add_to_loc = 0;
            }
        }
        // we only have "OPCODE, OPERAND" present (ex: LDA ZERO)
        else if(operand.empty())
        {
            /*
             * There is also a weird possibility of having an instruction of the kind "LEX RSUB", which is technically
             * okay to use, but if you actually use it, I think you are not functioning properly. Because who the hell
             * would use a label for an RSUB?
             *
             * So, I have not included bug fixing for this issue as of now.
            */

            operand = opcode;
            opcode = label;

            bool valid_op_code = op_tab.find(opcode);
            if (valid_op_code)
            {
                add_to_loc = 3;

                std::cout << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                          << operand << std::endl;
                intermediate_file << std::hex << LOC_CTR << '\t' << '\t' << opcode << '\t'
                                  << operand << std::endl;
            }
            else
            {
                std::cout << "ERROR, invalid opcode: " << opcode << " found\n";
                intermediate_file << "ERROR, invalid opcode: " << opcode << " found\n";

                add_to_loc = 0;
                // continue;
            }
        }

        LOC_CTR += add_to_loc;
    }
    
    /* check below later, as I needed to do this ONLY because I added 1 to the "END" label occurrence. I think we can
     * safely neglect that?
     * 
     * closing the input file below, as it is not required for the 2nd pass.
    */

    LOC_CTR -= 1;
    input_file.close();
    
    sym_tab.print();
    std::cout << "\n\n";

    /*
     * close the intermediate file to which we were writing to, and then re-open it in read mode.
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

    bool found_first = false;
    size_t space_taken = 0;
    line.clear();

    /*
     * this flag is to allow for not initialising the text record with the START and END records
     *
     * For now, I have just said "to continue" if such a record is found, but i think adding a variable to keep
     * track is much better for code maintenance, so might add it up later!
    */

    bool ignore;

    bool loop_check;
    while(std::getline(intermediate_file, line))
    {
        // ignore = false;
        loop_check = false;
        std::istringstream iss(line);

        opcode.clear(), operand.clear(), label.clear(), loc.clear();

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
                    std::cout << "couldn't find the LABEL_ADDRess for" + operand;
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

                // make the above stuff of length 6
                while(opcode_string.length() < 2)
                    opcode_string = '0' + opcode_string;

                while(label_string.length() < 4)
                    label_string = '0' + label_string;

                // need to handle care of the 'x' bit for indexing mode later

                /*
                 * handle the text record's length by making sure that THE CURRENT instruction can fit in the CURRENT
                 * text record.
                 *      If it can, go ahead and fill it in the CURRENT record.
                 *      Else, write the current record to the file, then re-initialize a new text record.
                */

                if(space_taken + 3 <= 30)
                    put_next += '_' + (opcode_string + label_string), space_taken += 3;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << space_taken;
                    std::string space_taken_string = sp.str();

                    while(space_taken_string.length() < 2)
                        space_taken_string = '0' + space_taken_string;

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    std::cout << text_record << std::endl;
                    obj_file << text_record << std::endl;

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + (opcode_string + label_string);
                    space_taken = 3;
                    found_first = false;
                }

                // std::cout << opcode_string << ' ' << label_string << ' ' << put_next << '\n';
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
                        token += std::to_string(static_cast<int>(operand[i]));
                        i++;
                    } while (operand[i] != '\'');

                    // using a temp string token for much better readability, to allow checks later when space_taken
                    // checks are made

                    // "operand.length() - 3" because we need to neglect the C, ' and '

                    if( (space_taken + operand.length() - 3) <= 30)
                        put_next += '_' + token, space_taken += (operand.length() - 3);
                    else
                    {
                        // write the current record, and re-initialize
                        std::ostringstream sp;
                        sp << std::hex << space_taken;
                        std::string space_taken_string = sp.str();

                        while(space_taken_string.length() < 2)
                            space_taken_string = '0' + space_taken_string;

                        text_record.append(space_taken_string);
                        text_record.append(put_next);

                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;

                        text_record.clear();
                        put_next.clear();

                        put_next = '_' + token;
                        space_taken = operand.length() - 3;
                        found_first = false;
                    }
                }
                else if (operand[0] == 'X')
                {
                    /* ex: 42069 VAL BYTE X'1b'
                     *
                     * Currently, this accepts hex consts only as a pair of numbers, i.e, the length is even.
                     * Not a big deal to change it to accept odd lengths too, so will change it later.
                    */

                    /* reuse the string created "token" in the same scope, instead of a new one. MFW I don't want to
                     * see clang-tidy warning lmao
                    */

                    token.clear();

                    // n -> NUMBER of chars to be collected, NOT the last index to be retreived
                    token = operand.substr(2, operand.length() - 3);

                    while(token.length() < 6)
                        token = '0' + token;

                    // length of the hex stuff in bytes is " ( operand.length() - 3 ) / 2 "

                    if( (space_taken + (operand.length() - 3) / 2) <= 30)
                        put_next += '_' + token, space_taken += (operand.length() - 3);
                    else
                    {
                        // write the current record, and re-initialize
                        std::ostringstream sp;
                        sp << std::hex << space_taken;
                        std::string space_taken_string = sp.str();

                        while(space_taken_string.length() < 2)
                            space_taken_string = '0' + space_taken_string;

                        text_record.append(space_taken_string);
                        text_record.append(put_next);

                        std::cout << text_record << std::endl;
                        obj_file << text_record << std::endl;

                        text_record.clear();
                        put_next.clear();

                        put_next = '_' + token;
                        space_taken = (operand.length() - 3) / 2;
                        found_first = false;
                    }
                }
            } // end opcode == "BYTE"
            else if (opcode == "WORD")
            {
                // ex: 1000 COUNTER WORD 256

                //loop_check = true;

                std::string token{};

                /* Need to convert the operand into a hex value.
                 *
                 * So, I first take in that integer (present as a string in "operand" object), and then use the std::hex
                 * manipulator on the ostringstream object to convert it to a hex string.
                */

                std::istringstream in(operand);
                int val;
                in >> val;

                std::ostringstream ou;
                ou << std::hex << val;
                token = ou.str();


                // do we need to clean this? IDTS because scope dies, and new object is formed in the next iteration.
                ou.str("");
                ou.clear();

                while(token.length() < 6)
                    token = '0' + token;

                if(space_taken + 3 <= 30)
                    put_next += '_' + token, space_taken += 3;
                else
                {
                    // write the current record, and re-initialize
                    std::ostringstream sp;
                    sp << std::hex << space_taken;
                    std::string space_taken_string = sp.str();

                    while(space_taken_string.length() < 2)
                        space_taken_string = '0' + space_taken_string;

                    text_record.append(space_taken_string);
                    text_record.append(put_next);

                    std::cout << text_record << std::endl;
                    obj_file << text_record << std::endl;

                    text_record.clear();
                    put_next.clear();

                    put_next = '_' + token;
                    space_taken = 3;
                    found_first = false;
                }

            } // end opcode == "WORD"
            else if(opcode == "RESW" || opcode == "RESB")
                continue;
        }
        else if(operand.empty() && opcode.empty())
        {
            /*
             * The only such instruction that exists in SIC is RSUB. It does not take in any operands, just opcode.
             *
             * In this case, we read the 'location' into "loc" variable, and the 'RSUB' into the "label" variable
             * because of how the code is structured.
             *
             * But, "END" can also enter this scope, and hence I check before if IT IS "END", and just continue
             * looping back if it is.
            */

            // std::cout << "\nRSUB found!\n"; // debug statement
            opcode = label;

            if(opcode == "END")
                continue;

            int OPC_CODE = op_tab.get_code(opcode);
            if(OPC_CODE == INT_MIN)
            {
                std::cerr << "couldn't find the OPC_CODE for" + opcode;
                exit(0);
            }

            // convert OPC_CODE that we got to a string, first converting to hex
            std::string opcode_string{};
            std::ostringstream okk;
            okk << std::hex << OPC_CODE;
            opcode_string = okk.str();

            okk.str("");
            okk.clear();

            while(opcode_string.length() < 2)
                opcode_string = '0' + opcode_string;

            if(space_taken + 3 <= 30)
                put_next += '_' + (opcode_string + "0000"), space_taken += 3;
            else
            {
                // write the current record, and re-initialize

                std::ostringstream sp;
                sp << std::hex << space_taken;
                std::string space_taken_string = sp.str();

                while(space_taken_string.length() < 2)
                    space_taken_string = '0' + space_taken_string;

                text_record.append(space_taken_string);
                text_record.append(put_next);

                std::cout << text_record << std::endl;
                obj_file << text_record << std::endl;

                text_record.clear();
                put_next.clear();

                put_next = '_' +(opcode_string + "0000");
                space_taken = 3;
                found_first = false;
            }
        }
        else if(operand.empty())
        {
            /*
             * ex:
             *                loc   label   opcode
             * something like 1000   LDA    LENGTH (without any label)
            */

            operand = opcode;
            opcode = label;

            // No need to find again if the opcode is valid or not. I'm lazy, and hey it works

            int OPC_CODE = op_tab.get_code(opcode);
            int LABEL_ADDR = sym_tab.get_code(operand);

            if(OPC_CODE == INT_MIN)
            {
                std::cerr << "couldn't find the OPC_CODE for" + opcode;
                exit(0);
            }

            if(LABEL_ADDR == INT_MIN)
            {
                std::cerr << "couldn't find the LABEL_ADDR for " + operand << "\n" + opcode << "-" << loc;
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

            // make the above stuff of length 6
            while(opcode_string.length() < 2)
                opcode_string = '0' + opcode_string;

            while(label_string.length() < 4)
                label_string = '0' + label_string;

            // need to handle care of the 'x' bit for indexing mode later

            /*
             * handle the text record's length by making sure that THE CURRENT instruction can fit in the CURRENT
             * text record.
             *      If it can, go ahead and fill it in the CURRENT record.
             *      Else, write the current record to the file, then re-initialize a new text record.
            */

            if(space_taken + 3 <= 30)
                put_next += '_' + (opcode_string + label_string), space_taken += 3;
            else
            {
                // write the current record, and re-initialize
                std::ostringstream sp;
                sp << std::hex << space_taken;
                std::string space_taken_string = sp.str();

                while(space_taken_string.length() < 2)
                    space_taken_string = '0' + space_taken_string;

                text_record.append(space_taken_string);
                text_record.append(put_next);

                std::cout << text_record << std::endl;
                obj_file << text_record << std::endl;

                text_record.clear();
                put_next.clear();

                put_next = '_' + (opcode_string + label_string);
                space_taken = 3;
                found_first = false;
            }
        }
        /*
         * this works flawlessly, in the sense that we have all the four variables filled for "START".
         * So we don't need to assign something else to opcode before checking. Does that make sense? I hope it does.
        */
        else if(opcode == "START")
            continue;

        /*
         * This is VERY important. This is going to initialise the "T_{location}" part for a new Text-Record
         * everytime we come out with "found_first" variable as "false"
        */

        if(!found_first)
        {
            loc = "00" + loc;
            text_record.append("T_" + loc + '_');
            // obj_file << loc << '_' << 30 << '_';
            found_first = true;
        }
    }

    std::ostringstream sp;
    sp << std::hex << space_taken;
    std::string space_taken_string = sp.str();

    while(space_taken_string.length() < 2)
        space_taken_string = '0' + space_taken_string;

    text_record.append(space_taken_string);
    text_record.append(put_next);

    sp.str("");
    sp.clear();

    std::cout << text_record << std::endl;
    obj_file << text_record << std::endl;


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