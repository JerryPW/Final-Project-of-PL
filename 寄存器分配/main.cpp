//
//  main.cpp
//  寄存器分配
//
//  Created by 李昊 on 2023/11/27.
//

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

struct var
{
    int v;
    var *next;
};

struct command
{
    string com_str; // 原始命令
    var *use;
    var *def;
    var *liveness;
};

struct Block
{
    command *program;
    command *end_info;
    // commmand *program_tail;
    int parent;
};

struct Program
{
    string program[100];
    Block *program_block;
};



int main()
{
    
    Program *prog;

    ifstream ifs;
    ifs.open("/Users/lihao/Documents/Programming_Language/寄存器分配/exp1.txt", ios::in);
    
    if (ifs.is_open())
    {
        string line;
        int i = 0;
        while (getline(ifs, line))
        {
            prog->program[i] = line;
            i++;
        }
        prog->program[i] = '\0';
    }
    else
        cout << "failed" << endl;
}

