//
//  main.cpp
//  寄存器分配
//
//  Created by 李昊 on 2023/11/27.
//

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
using namespace std;


struct command
{
    string com_str; // 原始命令
    unordered_set <int> use;
    unordered_set <int> def;
    unordered_set <int> liveness;
};

struct Block
{
    command program[100];
    // command end_info;
    Block *parent;
    int com_num;
};

struct Program
{
    string program[100];
    Block program_block[100];
    int block_num;
};

void use_def_calculate(command* com) {
    unordered_set<int> useSet;

    istringstream iss(com->com_str);
    string token;

    while (iss >> token) {
        if (token[0] == '#')
        {
            int var = stoi(token.substr(1));
            useSet.insert(var);
        }
        else if (token == "=")
        {
            for (int var : useSet) {
                com->def.insert(var);
            }
            useSet.clear();
        }
        else if (token == "*")
        {
            iss >> token;
            if (token[0] == '#' && token.size() > 1) {
                int var = stoi(token.substr(1));
                com->use.insert(var);
            }
            
        }
        else if (token == "jmp")
            continue;
        else if (token == "if")
        {
            while(iss >> token && token != "jmp" && token != "then" && token != "else")
            {
                for (int i = 0; i < token.size(); ++i) {
                    if (token[i] == '#')
                    {
                        int j = i + 1;
                        while (j < token.size() && isdigit(token[j]))
                            ++j;
                        int var = stoi(token.substr(i + 1, j - i - 1));
                        com->use.insert(var);
                        i = j - 1;
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < token.size(); ++i) {
                if (token[i] == '#')
                {
                    int j = i + 1;
                    while (j < token.size() && isdigit(token[j]))
                        ++j;
                    int var = stoi(token.substr(i + 1, j - i - 1));
                    com->use.insert(var);
                    i = j - 1;
                }
            }
        }
    }
    
    for (int var : useSet)
        com->use.insert(var);
}


void build_block(struct Program* prog)
{
    int i = 0; // Program
    int j = 0; // Block
    int k = 0; // Command
    while (prog->program[i] != "EOF")
    {
        if (prog->program[i].substr(0, 5) != "Block")
        {
            command com;
            com.com_str = prog->program[i];
            prog->program_block[j - 1].program[k] = com;
            prog->program_block[j - 1].com_num++;
            k++;
        } else{
            prog->program_block[j].com_num = 0;
            j++;
            k = 0;
        }
        i++;
    }
    prog->block_num = j;
}



int main()
{
    
    Program *prog = new Program;

    ifstream ifs;
    ifs.open("output.txt", ios::in);


    if (ifs.is_open())
    {
        string line;
        int i = 0; 
        while (getline(ifs, line))
        {
            // cout<<line<<endl;
            prog->program[i] = line;
            i++;
        }
        prog->program[i] = "EOF";
    }
    else
        cout << "failed" << endl;
    
    build_block(prog);

    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
            command myCommand = prog->program_block[block_id].program[com_id];
            use_def_calculate(&myCommand);
            cout << "use: ";
            for (int var : myCommand.use)
                cout << "#" << var << ' ' ;
            cout << endl;
            cout << "def: ";
            for (int var : myCommand.def)
                cout << "#" << var << ' ';
            cout << endl;
        }
    }
    return 0;
}

