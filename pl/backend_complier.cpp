#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include "rv_assemble.hpp"
using namespace std;


struct command
{
    string com_str; // 原始命令
    int address;
    op func;
    unordered_set <int> use;
    unordered_set <int> def;
    unordered_set <int> liveness;
};

struct Block
{
    command program[100];
    // command end_info;
    int start_address;
    vector <int> parent;
    int com_num;
    bool checked;

    Block()
    {
        com_num=0;
    }
};

struct Program
{
    vector<string> program;
    Block program_block[100];
    int block_num;
    Program()
    {
        block_num = 0;
    }
};

void record_block_parents(struct Program* prog)
{
    int i = 0;
    while (i < prog->block_num)
    {
        Block current_block = prog->program_block[i];
        command end_info = current_block.program[current_block.com_num - 1];
        istringstream iss(end_info.com_str);
        string token;
        unordered_set <int> destination;
        while (iss >> token)
        {
            if (token == "jmp")
            {
                iss >> token;
                int dest = stoi(token);
                destination.insert(dest);
            }
        }
        
        for (int dest : destination)
            prog->program_block[dest].parent.push_back(i);
        
        i++;
    }
}
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
                com->liveness.insert(var);
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
                        com->liveness.insert(var);
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
                    com->liveness.insert(var);
                    i = j - 1;
                }
            }
        }
    }
    
    for (int var : useSet) {
        com->use.insert(var);
        com->liveness.insert(var);
    }
}


void build_block(struct Program* prog)
{
    int i = 0; // Program
    int block_num = 0; // Block
    int k = 0; // Command
    int block_id = 0;
    int pc = 0;
    while (prog->program[i] != "EOF")
    {
        if (prog->program[i].substr(0, 5) != "Block")
        {
            command com;
            com.com_str = prog->program[i];
            com.address = pc;
            prog->program_block[block_id].program[k] = com;
            prog->program_block[block_id].com_num++;
            k++;
            pc += 4;
        } else{
            block_id = prog->program[i][6] - 48;
            prog->program_block[block_id].start_address = pc;
            prog->program_block[block_id].com_num = 0;
            block_num++;
            k = 0;
        }
        i++;
    }
    prog->block_num = block_num;
}


// int max_number(string line, int max)
// {
//     istringstream iss(line);
//     string token;
//     int var = -1;
//     while (iss >> token) {
//         if (token[0] == '#')
//         {
//             var = stoi(token.substr(1));
//         }
//         else if (token == "if")
//         {
//             while(iss >> token && token != "jmp" && token != "then" && token != "else")
//             {
//                 for (int i = 0; i < token.size(); ++i) {
//                     if (token[i] == '#')
//                     {
//                         int j = i + 1;
//                         while (j < token.size() && isdigit(token[j]))
//                             ++j;
//                         var = stoi(token.substr(i + 1, j - i - 1));
//                         i = j - 1;
//                     }
//                 }
//             }
//         }
//         else
//         {
//             for (int i = 0; i < token.size(); ++i) {
//                 if (token[i] == '#')
//                 {
//                     int j = i + 1;
//                     while (j < token.size() && isdigit(token[j]))
//                         ++j;
//                     var = stoi(token.substr(i + 1, j - i - 1));
//                     i = j - 1;
//                 }
//             }
//         }  
//         if (var > max)
//             max = var;
//     } 
//     return max;
// }

unordered_set <int> allocated_block;

void liveness_add(Program* prog, Block *block, command *pre_com, int now_block_id, int now_live_id, int now_var, int end_block_id, int end_com_id) {


    // cout<<"Debug: "<<now_block_id<<" "<<now_live_id<<" "<<now_var<<endl;

    if(!pre_com->def.count(now_var) && !((now_block_id == end_block_id) && (now_live_id == end_com_id))) {
        if(!pre_com->liveness.count(now_var))
            pre_com->liveness.insert(now_var);
        now_live_id--;
        if (now_live_id >= 1) {
            pre_com = &(block->program[now_live_id - 1]);
            liveness_add(prog, block, pre_com, now_block_id, now_live_id, now_var, end_block_id, end_com_id);
        } else {
            for(int i = 0; i < block->parent.size(); i++){
                int pre_block_id = block->parent[i];
                Block *pre_block = &(prog->program_block[block->parent[i]]);
                int pre_live_id = pre_block->com_num;
                pre_com = &(pre_block->program[pre_live_id - 1]);
                liveness_add(prog, pre_block, pre_com, pre_block_id, pre_live_id, now_var, end_block_id, end_com_id);
            }
        }
    }
}

void liveness_gen(struct Program* prog, int now_block_id, int now_com_id) {
    // cout<<now_block_id<<" "<<now_com_id<<endl;
    Block *now_block = &(prog->program_block[now_block_id]);
    command *now_com = &(now_block->program[now_com_id - 1]);
    command *pre_com  = new command;

    int end_com_id = now_com_id;
    int end_block_id = now_block_id;
    for (int now_var : now_com->liveness) {
        int now_live_id = now_com_id;
        now_live_id--;
        if (now_live_id >= 1) {
            pre_com = &(now_block->program[now_live_id - 1]);
            liveness_add(prog, now_block, pre_com, now_block_id, now_live_id, now_var, end_block_id, end_com_id);
        } else {
            for(int i = 0; i < now_block->parent.size(); i++){
                int pre_block_id = now_block->parent[i];
                Block *pre_block = &(prog->program_block[now_block->parent[i]]);
                int pre_live_id = pre_block->com_num;
                pre_com = &(pre_block->program[pre_live_id - 1]);
                liveness_add(prog, pre_block, pre_com, pre_block_id, pre_live_id, now_var, end_block_id, end_com_id);
            }
        }
    }

    // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<" "<<prog->block_num - 1<<endl;
    //   0  1  5  5

    if(now_block_id != 0) {
        // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<" "<<prog->block_num - 1<<endl;
        if(now_com_id > 1) {
            now_com_id--;
            liveness_gen(prog, now_block_id, now_com_id);
        } else{
            allocated_block.insert(now_block_id);
            for(int i = 0; i < now_block->parent.size();i++) {
                now_block_id = now_block->parent[i];
                // cout<< now_block_id << endl;
                now_com_id = prog->program_block[now_block_id].com_num;
                if(allocated_block.size() != (prog->block_num - 1))
                    liveness_gen(prog, now_block_id, now_com_id);
            }
        }
    } else {
        // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<" "<<prog->block_num - 1<<endl;
        if(now_com_id > 1) {
            now_com_id--;
            liveness_gen(prog, now_block_id, now_com_id);
        }else {
            // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<" "<<prog->block_num - 1<<endl;
            if (allocated_block.size() == (prog->block_num - 1)){
                // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<" "<<prog->block_num - 1<<endl;
                return;
            } else {
                // cout<<now_block_id<<" "<<now_com_id<<" "<<allocated_block.size()<<endl;
                int max_unallocated = 0;
                for (int a = 0; a < prog->block_num; a++) {
                    if(!allocated_block.count(a)) 
                        max_unallocated = a;
                }
                // cout<<max_unallocated<<endl;
                int next_block_id = max_unallocated;
                int next_com_id = prog->program_block[next_block_id].com_num;
                liveness_gen(prog, next_block_id, next_com_id);
            }
        }     
    }
    
    
} 

vector<int> wrong;
int place = 0;
struct Program* rewrite_program(struct Program* prog)
{
    struct Program* new_prog = new struct Program;

    for (int j = 0; j < wrong.size(); ++j)
    {
        if(wrong[j])
        {
            place += 16;
            wrong[j]=place;
        }
    }    

    for (int i = 0; i < prog->block_num; i++)
    {
        new_prog->program.push_back("Block " + to_string(i));
        struct Block temp_block = prog->program_block[i];
        
        
        for (int j = 0; j < temp_block.com_num; j++)
        {
            struct command temp_com = temp_block.program[j];
            for (int var : temp_com.use)
            {
                if (wrong[var])
                    new_prog->program.push_back("#" + to_string(var) + " = *(%rbp - " + to_string(wrong[var]) + ")");
            }
            new_prog->program.push_back(temp_com.com_str);
            for (int var : temp_com.def)
            {
                if (wrong[var])
                    new_prog->program.push_back("*(%rbp - " + to_string(wrong[var]) + ") = #" + to_string(var));
            }
        }
    }
    new_prog->program.push_back("EOF");
    return new_prog;
}

/*
变量采用数字存储
需要把初始程序的变量修改为数字
例如，x = x + 1 -> (0) = (0) + 1
*/
int var_cnt;


int reg_id[]= {
5,6,7,9,18,
19,20,21,22,23,
24,25,26,27,28,
29,30,31,
1,10,11,12,13,
14,15,16,17
};

int find_reg_id(int reg)
{
    for(int i=0;i<27;++i)
        if(reg_id[i]==reg)
            return i;
    return -1;
}

bool need_back_up[27]={0};
bool used_reg[27]={0};
bool once_used_reg[27]={0};
bool callee[]={
0,0,0,1,1, 
1, 1, 1, 1, 1, 
1, 1, 1, 1, 0, 
0, 0, 0,
0,0,0,0,0,
0,0,0,0
};

int * var_rg;

vector<bool> spilled;//一个变量不能被spill两次

struct move_command
{
    int v1, v2;
    bool coalesced, invalid;
};

vector <move_command> move_list;

bool *interference_graph;

void init()
{
    int i, j;
    for (i = 0; i < var_cnt; ++ i)
        for (j = 0; j < var_cnt; ++ j)
            interference_graph[i * var_cnt + j] = false;
}

void del_useless_move()
{
    for (int i = 0; i < move_list.size(); ++ i)
    {
        if (interference_graph[move_list[i].v1 * var_cnt + move_list[i].v2])
            move_list[i].invalid = true;
    }
}
bool *rg_choices;
int *degree;

bool *removed;

void get_degrees()
{
    int i, j;
    for (i = 0; i < var_cnt; ++i)
    {
        degree[i] = 0;
        if(!removed[i])
            for (j = 0; j < var_cnt; ++j)
                if (!removed[j] && i != j && interference_graph[i * var_cnt + j])
                    ++ degree[i];
    }
}

void get_move_info()
{
    for(int i = 0; i < var_cnt; ++ i)
        interference_graph[i * var_cnt + i] = false;
    int tmp;
    for (int cur = 0; cur < move_list.size(); ++ cur)
    {
        if (move_list[cur].invalid || move_list[cur].coalesced)
            continue;
        tmp = move_list[cur].v1;
        interference_graph[tmp * var_cnt + tmp] |= true;
        tmp = move_list[cur].v2;
        interference_graph[tmp * var_cnt + tmp] |= true;
    }
}

//用于判断公共邻居
bool *neighbors;

void init_neighbors()
{
    int i;
    for (i = 0; i < var_cnt; ++ i)
        neighbors[i] = false;
}

bool check_move(string cmd)
{
    int len = cmd.length();
    int eq = -1, v1 = 0, v2 = 0;
    for (int i = 0; i < len; ++ i)
        if (cmd[i] == '=')
        {
            if(eq != -1)
                return false;
            eq = i;
        }
    
    if (eq == -1)
        return false;
    bool v = false;
    int nd_cnt = 0;
    for (int i = eq + 1; i < len; ++ i)
    {
        if(cmd[i] == ' ')
            continue;
        if(cmd[i] == '#')
            v = true;
        if(cmd[i] >= '9' || cmd[i] <= '0')
            nd_cnt ++;
    }
    if(!v)
        return false;
    if (nd_cnt != 1)
        return false;
    return true;

}

vector<int> to;

int find(int x)
{
    if(x!=to[x])
        to[x]=find(to[x]);
    return to[x];
}

void combine(int x,int y)
{
    x=find(x),y=find(y),to[y]=x;
}

struct remove_stk
{
    int id;
    bool rm_type; // false for simplify, true for spill.
};
vector<remove_stk> rm_stk;
bool allocate(int rg_cnt)
{
    int res_cnt = var_cnt, i, deg_cnt, tmp;
    for (i = 0; i < var_cnt; ++ i)
        removed[i] = false;

    bool flag = false;

    //寄存器分配算法
    while(res_cnt)
    {
        //首先检查能否simplify
        flag = false;
        get_degrees();
        get_move_info();
        
        // for(int a = 0; a < var_cnt; ++ a)
        // {
        //     if(removed[a])
        //         continue;
        //     cout<<a<<" : "<<degree[a]<<" : "<<interference_graph[a * var_cnt + a]<<endl;
        //     for (int b = 0; b < var_cnt; ++b)
        //     {
        //         if(!removed[b] && interference_graph[a * var_cnt + b] && b != a)
        //             cout<<b<<" ";
        //     }
        //     cout<<endl;
        // }

        for (i = 0; i < var_cnt; ++ i)
        {
            if (!removed[i] && !interference_graph[i * var_cnt + i] && degree[i] < rg_cnt)
            {
                removed[i] = true;
                -- res_cnt;
                remove_stk now;
                now.id = i;
                now.rm_type = false;
                rm_stk.push_back(now);
                cout<<"simplify #"<<i<<endl; 
                flag = true;
                break;
            }
        }
        if(flag)
            continue;
        
        cout<<"Can not simplify anymore.\n";
        //保守合并
        
        for (int cur = 0; cur < move_list.size(); ++ cur)
        {
            if (move_list[cur].coalesced || move_list[cur].invalid)
                continue;
            // cout<<"move: "<<move_list[cur].v1<<" "<<move_list[cur].v2<<endl;
            init_neighbors();
            for (i = 0; i < var_cnt; ++ i)
            {   
                if(!removed[i] && i != move_list[cur].v1 && i != move_list[cur].v2)
                    if(interference_graph[i * var_cnt + move_list[cur].v1] && interference_graph[i * var_cnt + move_list[cur].v2])
                        neighbors[i] = true;
            }
            // for (i = 0; i < var_cnt; ++ i)
            //     cout<<i<<" "<<neighbors[i]<<endl;
            //策略1
            deg_cnt = 0;
            for (i = 0; i < var_cnt; ++ i)
                if(neighbors[i] && degree[i] >= rg_cnt)
                    ++deg_cnt;
            if (deg_cnt < rg_cnt)
            {
                //修改v1和v2的邻居
                combine(move_list[cur].v1,move_list[cur].v2);
                cout<<"Coalesce: #"<<move_list[cur].v1<<" <= #"<<move_list[cur].v2<<endl;
                for (i = 0; i < var_cnt; ++ i)
                    if(i!= move_list[cur].v2 && interference_graph[i * var_cnt + move_list[cur].v2])
                    {
                        interference_graph[i * var_cnt + move_list[cur].v1] = true;
                        interference_graph[move_list[cur].v1 * var_cnt + i] = true;
                    }
                //删除v2的信息
                removed[move_list[cur].v2] = true;
                move_list[cur].coalesced = true;
                -- res_cnt;
                flag = true;
                break;
            }

            //策略2
            for (i = 0; i < var_cnt; ++ i)
                if(!removed[i] && i != move_list[cur].v1 && interference_graph[i * var_cnt + move_list[cur].v1])//i是v1的邻居
                    if(!neighbors[i] && degree[i] >= rg_cnt)
                    {
                        flag = true;
                        break;   
                    }
            if (!flag)
            {
                //修改v1和v2的邻居
                combine(move_list[cur].v1,move_list[cur].v2);
                cout<<"Coalesce: #"<<move_list[cur].v1<<" => #"<<move_list[cur].v2<<endl;
                for (i = 0; i < var_cnt; ++ i)
                    if(i!= move_list[cur].v1 && interference_graph[i * var_cnt + move_list[cur].v1])
                    {
                        interference_graph[i * var_cnt + move_list[cur].v2] = true;
                        interference_graph[move_list[cur].v2 * var_cnt + i] = true;
                    }

                removed[move_list[cur].v1] = true;
                move_list[cur].coalesced = true;
                -- res_cnt;
                flag = true;
                break;             
            }
            flag = false;
            for (i = 0; i < var_cnt; ++ i)
                if(!removed[i] && i != move_list[cur].v2 && interference_graph[i * var_cnt + move_list[cur].v2])
                    if(!neighbors[i] && degree[i] >= rg_cnt)
                    {
                        flag = true;
                        break;   
                    }
            if (!flag)
            {
                //修改v1和v2的邻居
                combine(move_list[cur].v1,move_list[cur].v2);
                cout<<"Coalesce: #"<<move_list[cur].v1<<" <= #"<<move_list[cur].v2<<endl;
                for (i = 0; i < var_cnt; ++ i)
                    if(i!= move_list[cur].v2 && interference_graph[i * var_cnt + move_list[cur].v2])
                    {
                        interference_graph[i * var_cnt + move_list[cur].v1] = true;
                        interference_graph[move_list[cur].v1 * var_cnt + i] = true;
                    }

                removed[move_list[cur].v2] = true;
                move_list[cur].coalesced = true;
                -- res_cnt;
                flag = true;
                break;          
            }
            flag = false;
        }

        if(flag)//有合并行为
            continue;

        //Freeze
        for (int cur = 0; cur < move_list.size(); ++ cur)
        {
            if (move_list[cur].coalesced || move_list[cur].invalid)
                continue;
            cout<<"freeze: "<<move_list[cur].v1<<" = "<<move_list[cur].v2<<endl;
            move_list[cur].invalid = true;
            flag = true;
            break;
        }
        if(flag)
            continue;

        //Spill
        for (i = 0; i < var_cnt; ++ i)
        {
            if(!removed[i] && !spilled[i])
            {
                cout<<"spill #"<<i<<endl; 
                removed[i] = true;
                -- res_cnt;
                remove_stk now;
                now.id = i;
                now.rm_type = true;
                rm_stk.push_back(now);  
                break;
            }
        } 
    }
    
    //开始分配寄存器

    flag = false;

    for(int cur = rm_stk.size() - 1; cur >= 0; -- cur)
    {        
        tmp = rm_stk[cur].id;
        for (i = 0; i < rg_cnt; ++ i)
            rg_choices[i] = true;
        for (i = 0; i< var_cnt; ++ i)
            if(!removed[i] && i != tmp && interference_graph[i * var_cnt + tmp] && var_rg[i] != -1)
                rg_choices[var_rg[i]] = false;
        
        //尝试分配
        for (i = 0; i < rg_cnt; ++ i)
            if (rg_choices[i])
            {
                var_rg[tmp] = i;
                break;
            }
        if(var_rg[tmp] != -1)//分配成功
            removed[tmp] = false;//把该点加回图中
        else
            flag = true;//存在真spill

    }
    //根据var_rg的信息即可知道哪些变量是真spill
    for (i = 0; i < var_cnt; ++ i)
    {
        // cout<<i<<" "<<find(i)<<endl;
        if(var_rg[find(i)]==-1)
            var_rg[find(i)]=var_rg[i];
    }
    for (i = 0; i < var_cnt; ++ i)
    {
        if(var_rg[i]==-1)
            var_rg[i]=var_rg[find(i)];
    }
    
    for(i = 0; i < var_cnt; ++ i)
    {
        if(var_rg[i]!=-1)
            var_rg[i] = reg_id[var_rg[i]];
    }
    for(i = 0; i < var_cnt; ++ i)
    {
        cout<<"Reg info: #"<<i<<" <- X"<<var_rg[i]<<endl;
    }

    if(flag)
        return false;
    return true;
}

void get_op(command* com)
{
    istringstream iss(com->com_str);
    string token;

    while (iss >> token) {
        if (token.find("read_int") != std::string::npos || token.find("write_int") != std::string::npos || token.find("func") != std::string::npos)
            com->func = FUNC;
        else if (token.find("MUL") != std::string::npos)
            com->func = MUL;
        else if (token.find("PLUS") != std::string::npos)
            com->func = ADD;
        else if (token.find("MINUS") != std::string::npos)
            com->func = SUB;
        else if (token.find("DIV") != std::string::npos)
            com->func = DIV;
        else if (token.find("REM") != std::string::npos)
            com->func = REM;
        else if (token.find("AND") != std::string::npos)
            com->func = AND;
        else if (token.find("OR") != std::string::npos)
            com->func = OR;
        else if (token.find("NOT") != std::string::npos)
            com->func = NOT;
        else if (token.find("UMINUS") != std::string::npos)
            com->func = USUB;
        else if (token.find("DEREF") != std::string::npos)
            com->func = DEREF;
        else if (token.find("if") != std::string::npos) {
            com->func = IF;
        } else if (token.find("*") != std::string::npos) {
            com->func = ST;
        } else if (token.find("%rbp") != std::string::npos) {
            if (com->use.size() == 0)
                com->func = LB;
            else 
                com->func = SB;
        }
        
    }
}

void assemble_gen_print(command *com) {
    if(com->func == FUNC) {
        if(com->com_str.find("=") != std::string::npos){
            auto rd = *(com->def.begin());
            size_t s = com->com_str.find("=") + 2;
            size_t l = com->com_str.find("(") - s;
            string b = com->com_str.substr(com->com_str.find("= ") + 2);
            istringstream iss(b);
            string token;
            int now_reg_id = 0;
            while (iss >> token) {
                if (token.back() == ')') {
                    token.pop_back(); 
                }
                if (token.find("#") != string::npos) {
                    int c = stoi(token.substr(token.find("#")+1, token.find(",") - token.find("#")-1));
                    cout << "add x" <<arg_reg[now_reg_id]<<", x"<<var_rg[c]<<", x0"<<endl;
                    now_reg_id++;
                } else if (token.find("$(") != string::npos) {
                    cout << "addi x" <<arg_reg[now_reg_id]<<", #"<<token.substr(token.find("$(")+2, token.find(")") - token.find("$(")-2)<<", x0"<<endl;
                    now_reg_id++;
                }
            }
            cout<<opName[com->func]<<" "<<com->com_str.substr(s, l)<<endl;
            cout<<opName[0]<<" "<<"x"<<var_rg[rd]<<", "<<"x0"<<", "<<"x1"<<endl;
        }else {
            string b = com->com_str.substr(com->com_str.find("(") + 1);
            istringstream iss(b);
            string token;
            int now_reg_id = 0;
            while (iss >> token) {
                if (token.back() == ')') {
                    token.pop_back(); 
                }
                if (token.find("#") != string::npos) {
                    int c = stoi(token.substr(token.find("#")+1, token.find(",") - token.find("#")-1));
                    cout << "add x" <<arg_reg[now_reg_id]<<", x"<<var_rg[c]<<", x0"<<endl;
                    now_reg_id++;
                } else if (token.find("$(") != string::npos) {
                    cout << "addi x" <<arg_reg[now_reg_id]<<", #"<<token.substr(token.find("$(")+2, token.find(")") - token.find("$(")-2)<<", x0"<<endl;
                    now_reg_id++;
                }
            }
            size_t l = com->com_str.find("(");
            cout<<opName[com->func]<<" "<<com->com_str.substr(0, l)<<endl;
        }
    } else if (com->func == ADD || com->func == MUL || com->func == SUB || com->func == AND || com->func == REM || com->func == DIV || com->func == OR) {
        auto rd = *(com->def.begin());
        if (com->use.size() == 0){
            size_t s1 = com->com_str.find("($(");
            if(s1 != std::string::npos) {
                s1 += 3;
                size_t l1 = com->com_str.find("),") - s1;
                size_t s2 = com->com_str.find(" $(") + 3;
                size_t l2 = com->com_str.find("))") - s2;
                cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"#"<<com->com_str.substr(s1, l1)<<", "<<"#"<<com->com_str.substr(s2, l2)<<endl;
            }else{
                size_t s = com->com_str.find("$(") + 2;
                size_t l = com->com_str.find(")") - s;
                cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x0"<<", "<<"#"<<com->com_str.substr(s, l)<<endl;
            }
        }else if (com->use.size() == 1) {
            auto rs1 = *(com->use.begin());
            size_t s = com->com_str.find("$(");
            if(s != std::string::npos){
                s += 2;
                size_t l = com->com_str.find("))") - s;
                cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"#"<<com->com_str.substr(s, l)<<endl;
            }else{
                cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x0"<<endl;
            }
        }else if (com->use.size() == 2) {
            auto rs1 = *(com->use.begin());
            int rs2 = 0;
            for(int r : com->use){
                rs2 = r;
            }
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x"<<var_rg[rs2]<<endl;
        }
    } else if (com->func == NOT) {
        auto rd = *(com->def.begin());
        if (com->use.size() == 0){
            size_t s = com->com_str.find("$(") + 2;
            size_t l = com->com_str.find("))") - s;
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", !"<<"#"<<com->com_str.substr(s, l)<<endl;
        }else if (com->use.size() == 1) {
            auto rs1 = *(com->use.begin());
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", !"<<"x"<<var_rg[rs1]<<endl;
        }
    } else if (com->func == USUB) {
        auto rd = *(com->def.begin());
        if (com->use.size() == 0){
            size_t s = com->com_str.find("$");
            size_t l = com->com_str.find("))") - s;
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", x0, "<<"#"<<com->com_str.substr(s, l)<<endl;
        }else if (com->use.size() == 1) {
            auto rs1 = *(com->use.begin());
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", x0, "<<"x"<<var_rg[rs1]<<endl;
        }
    } else if (com->func == LB) {
        auto rd = *(com->def.begin());
        size_t s = com->com_str.find("- ") + 2;
        size_t l = com->com_str.find(")") - s;
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", -"<<com->com_str.substr(s, l)<<"(%rbp)"<<endl;
    } else if (com->func == SB) {
        auto rd = *(com->use.begin());
        size_t s = com->com_str.find("- ") + 2;
        size_t l = com->com_str.find(")") - s;
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", -"<<com->com_str.substr(s, l)<<"(%rbp)"<<endl;
    } else if (com->func == IF) {
        istringstream iss(com->com_str);
        string token;
        string thenOperation;
        string elseOperation;
        string condition;
        while (iss >> token) {
            if (token == "if") {
                while (iss >> token && token != "then") {
                    condition += token + " ";
                }

                while (iss >> token && token != "else") {
                    thenOperation += token + " ";
                }

                while (iss >> token) {
                    elseOperation += token + " ";
                }
                break; 
            }
        }
        if(com->use.size() == 1){
            auto rs = *(com->use.begin());
            if(condition.find(",") == std::string::npos){
                cout<<"bne"<<" "<<"x"<<var_rg[rs]<<", x0"<<" "<<"Else"<<endl;
                size_t find = thenOperation.find("jmp ");
                char end = thenOperation[find+4];
                cout<<"jal Block "<<end<<endl;
                cout<<"Else:"<<endl;
                size_t find_two = elseOperation.find("jmp ");
                char end_two = elseOperation[find_two+4];
                cout<<"jal Block "<<end_two<<endl;
            } else {
                size_t s = com->com_str.find("$(") + 2;
                size_t l = com->com_str.find("))") - s;
                cout<<"b"<<char(condition[1]+32)<<char(condition[2]+32)<<" "<<"x"<<var_rg[rs]<<", #"<<com->com_str.substr(s, l)<<" "<<"Else"<<endl;
                size_t find = thenOperation.find("jmp ");
                char end = thenOperation[find+4];
                cout<<"jal Block "<<end<<endl;
                cout<<"Else:"<<endl;
                size_t find_two = elseOperation.find("jmp ");
                char end_two = elseOperation[find_two+4];
                cout<<"jal Block "<<end_two<<endl;
            }
        }else if(com->use.size() == 0) {
            size_t s1 = com->com_str.find("($(") + 3;
            size_t l1 = com->com_str.find("),") - s1;
            size_t s2 = com->com_str.find(" $(") + 3;
            size_t l2 = com->com_str.find(")))") - s2;
            cout<<"b"<<char(condition[1]+32)<<char(condition[2]+32)<<" #"<<com->com_str.substr(s1, l1)<<", #"<<com->com_str.substr(s2, l2)<<" "<<"Else"<<endl;
            size_t find = thenOperation.find("jmp ");
            char end = thenOperation[find+4];
            cout<<"jal Block "<<end<<endl;
            cout<<"Else:"<<endl;
            size_t find_two = elseOperation.find("jmp ");
            char end_two = elseOperation[find_two+4];
            cout<<"jal Block "<<end_two<<endl;
            
        } else if(com->use.size() == 2) {
            auto rs1 = *(com->use.begin());
            int rs2 = 0;
            for(int r : com->use){
                rs2 = r;
            }
            cout<<"b"<<char(condition[1]+32)<<char(condition[2]+32)<<" "<<"x"<<var_rg[rs1]<<", "<<"x"<<var_rg[rs2]<<" "<<"Else"<<endl;
            size_t find = thenOperation.find("jmp ");
            char end = thenOperation[find+4];
            cout<<"jal Block "<<end<<endl;
            cout<<"Else:"<<endl;
            size_t find_two = elseOperation.find("jmp ");
            char end_two = elseOperation[find_two+4];
            cout<<"jal Block "<<end_two<<endl;
            
        }
    } else if (com->func == JAL){
        cout<<opName[com->func]<<" Block "<<com->com_str[com->com_str.find("jmp ")+4]<<endl;
    } else if (com->func == DEREF){
        auto rd = *(com->def.begin());
        auto rs = *(com->use.begin());
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", 0(x"<<var_rg[rs]<<")"<<endl;
    } else if(com->func == ST){
        auto rs1 = *(com->use.begin());
        int rs2 = 0;
        for(int r : com->use){
            rs2 = r;
        }
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rs1]<<", 0(x"<<var_rg[rs2]<<")"<<endl;
    }

}


// void assemble_gen(Program* prog) {
//     for(int block_id = 0; block_id < prog->block_num; block_id++) {
//         cout<<"Block "<<block_id<<":"<<endl;
//         for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {


//             command *com = &(prog->program_block[block_id].program[com_id]);

//             cout<<com->com_str<<", "<<opName[com->func]<<" : ";
//             assemble_gen_print(com);
//             // cout<<com->com_str<<" "<<opName[com->func]<<endl;
//         }
//     }
// }

struct reg_back_up
{
    //a是需要备份的寄存器，b是备份到的地方
    //如果address是false意味着a,b都是寄存器的编号
    //否则那么a是寄存器的编号，b是%rbp-place的place

    bool address;
    int a,b;
};

vector<reg_back_up> temp_info;

void get_caller_saved_reg_backup(command *cmd)
{
    temp_info.clear();

    //首先通过目标语句的liveness判断出当前可用的寄存器以及需要保存的寄存器
    for(int i=0;i<27;++i)
    {
        if(i>=18)
            need_back_up[i]=true;
        else
            need_back_up[i]=false;
        used_reg[i]=false;
    }
    for(int var : cmd->liveness)
    {
        int tmp_reg=var_rg[var];
        tmp_reg=find_reg_id(tmp_reg);
        used_reg[tmp_reg]=true;
        //只有live的caller-saved寄存器，和过程调用的参数需要备份
        if(!callee[tmp_reg])    
            need_back_up[tmp_reg]=true;
    }

    for(int i=0;i<27;++i)
    {
        if(need_back_up[i])
        {
            reg_back_up temp;
            temp.a=reg_id[i];
            bool find_reg=false;
            for(int j=0;j<18;++j)
            {
                if(callee[j]&&!used_reg[j])
                {
                    once_used_reg[j]=1;
                    used_reg[j]=1;
                    find_reg=true;
                    temp.b=reg_id[j];
                    temp.address=false;
                    break;
                }
            }
            if(!find_reg)
            {
                place+=16;
                temp.b=place;
                temp.address=true;
            }
            temp_info.push_back(temp);
        }
    }
}

void get_callee_saved_reg_backup()
{
    temp_info.clear();
    for(int i=0;i<18;++i)
        used_reg[i]=false;  
    
    for(int i=0;i<18;++i)
    {
        if(callee[i])
        {
            reg_back_up temp;
            temp.a=reg_id[i];
            bool find_reg=false;
            for(int j=0;j<18;++j)
            {
                if(!callee[j]&&!once_used_reg[j]&&!used_reg[j])
                {
                    used_reg[j]=1;
                    find_reg=true;
                    temp.b=reg_id[j];
                    temp.address=false;
                    break;
                }
            }
            if(!find_reg)
            {
                place+=16;
                temp.b=place;
                temp.address=true;
            }
            temp_info.push_back(temp);
        }
    }
}


struct call_str
{
    vector<string> call_out;
};

vector<call_str> caller_out;
vector<call_str> callee_out;

void caller_callee(Program* prog) 
{
    call_str ttemp;
    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
            command *com = &(prog->program_block[block_id].program[com_id]);
            // cout<<com->com_str<<", "<<opName[com->func]<<" : \n";
            if(com->func==FUNC)
            {
                ttemp.call_out.clear();
                get_caller_saved_reg_backup(com);
                for(int i = 0; i<temp_info.size(); ++i)
                {
                    bool add = temp_info[i].address;
                    int a = temp_info[i].a, b = temp_info[i].b;
                    if(add)
                        ttemp.call_out.push_back("sb x"+to_string(a)+", "+to_string(-b)+"(%rbp)");
                    else
                        ttemp.call_out.push_back("add x"+to_string(b)+", "+"x"+to_string(a)+", x0");
                }
                caller_out.push_back(ttemp);

                ttemp.call_out.clear();
                // get_caller_saved_reg_backup(com);
                for(int i = 0; i<temp_info.size(); ++i)
                {
                    
                    bool add = temp_info[i].address;
                    int a = temp_info[i].a, b = temp_info[i].b;
                    if(add)
                        ttemp.call_out.push_back("lb x"+to_string(a)+", "+to_string(-b)+"(%rbp)");
                    else
                        ttemp.call_out.push_back("add x"+to_string(a)+", "+"x"+to_string(b)+", x0");
                }
                caller_out.push_back(ttemp);
            }
        }
    }

    ttemp.call_out.clear();
    get_callee_saved_reg_backup();
    for(int i = 0; i<temp_info.size(); ++i)
    {
        bool add = temp_info[i].address;
        int a = temp_info[i].a, b = temp_info[i].b;
        if(add)
            ttemp.call_out.push_back("sb x"+to_string(a)+", "+to_string(-b)+"(%rbp)");
        else
            ttemp.call_out.push_back("add x"+to_string(b)+", "+"x"+to_string(a)+", x0");
    }
    callee_out.push_back(ttemp);

    ttemp.call_out.clear();
    for(int i = 0; i<temp_info.size(); ++i)
    {
        bool add = temp_info[i].address;
        int a = temp_info[i].a, b = temp_info[i].b;
        if(add)
            ttemp.call_out.push_back("sb x"+to_string(a)+", "+to_string(-b)+"(%rbp)");
        else
            ttemp.call_out.push_back("add x"+to_string(b)+", "+"x"+to_string(a)+", x0");
    }
    callee_out.push_back(ttemp);

}

void assemble_gen(Program* prog) {

    for(int i = 0; i<callee_out[0].call_out.size();++i)
        cout<<callee_out[0].call_out[i]<<endl;

    int temp = 0;

    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        cout<<"Block "<<block_id<<":"<<endl;
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
            command *com = &(prog->program_block[block_id].program[com_id]);
            cout<<com->com_str<<", "<<opName[com->func]<<" : \n";

            if(com->func==FUNC)
            {
                for(int i = 0; i<caller_out[temp].call_out.size();++i)
                    cout<<caller_out[temp].call_out[i]<<endl;
                ++temp;
            }

            assemble_gen_print(com);
            if(com->func==FUNC)
            {
                for(int i = 0; i<caller_out[temp].call_out.size();++i)
                    cout<<caller_out[temp].call_out[i]<<endl;
                ++temp;
            }

        }
    }
    cout<<"Block "<<prog->block_num<<":"<<endl;
    for(int i = 0; i<callee_out[1].call_out.size();++i)
        cout<<callee_out[1].call_out[i]<<endl;
}

int main()
{
    freopen("result.txt", "w", stdout);
    Program *prog = new Program;
    Program *tmp_prog=NULL;
    ifstream ifs;
    ifs.open("BasicBlock.txt", ios::in);

    int vars = -1;
    if (ifs.is_open())
    {
        string line;
        int i = 0;
        while (getline(ifs, line))
        {
            // vars = max_number(line, vars
            prog->program.push_back(line);
            i++;
        }
        prog->program.push_back("EOF");
    }
    else
        cout << "failed" << endl;
    // int RAX_NUMBER = vars + 1;
    // int i = 0;
    // while (prog->program[i] != "EOF")
    // {
    //     if (check_func(prog->program[i]))
    //     {
    //         string target = "read_int()";
    //         size_t pos = prog->program[i].find(target);
    //         prog->program[i].replace(pos, target.length(), "#" + to_string(RAX_NUMBER));
    //         string com = "#" + to_string(RAX_NUMBER) + " = read_int()";
    //         prog->program.insert(prog->program.begin() + i, com);
    //     }
    //     i++;
    // }  
    bool ok = false;
    bool first=true;
    do
    { 
        build_block(prog);
        
        for(int block_id = 0; block_id < prog->block_num; block_id++) {
            for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
                command *myCommand = &(prog->program_block[block_id].program[com_id]);
                use_def_calculate(myCommand);
            }
        }

        record_block_parents(prog);

        int end_block_id = prog->block_num - 1;
        int end_com_id = prog->program_block[end_block_id].com_num;
        liveness_gen(prog, end_block_id, end_com_id);

        // for(int block_id = 0; block_id < prog->block_num; block_id++) {
        //     cout<<"Block "<<block_id<<":"<<endl;
        //     for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
        //         command myCommand = prog->program_block[block_id].program[com_id];
        //         cout<<myCommand.com_str<<endl;
        //         cout << "def: ";
        //         for (int var : myCommand.def)
        //         {
        //             cout << "#" << var << ' ';
        //         }
        //         cout << "use: ";
        //         for (int var : myCommand.use)
        //         {
        //             cout << "#" << var << ' ';
        //         }
        //         cout << "liveness: ";
        //         for (int var : myCommand.liveness)
        //         {
        //             cout << "#" << var << ' ';
        //         }
        //         cout << endl << endl;
        //     }
        // }

        var_cnt = 0;

        for(int block_id = 0; block_id < prog->block_num; block_id++) {
            cout<<"Block "<<block_id<<":"<<endl;
            // cout<<"Com: "<<prog->program_block[block_id].com_num<<endl;
            for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
                command myCommand = prog->program_block[block_id].program[com_id];
                cout<<myCommand.com_str<<endl;

                if (check_move(myCommand.com_str))
                {
                    move_command now;
                    now.coalesced = false;
                    now.invalid = false;
                    int tmp;
                    for(int i : myCommand.def)
                        tmp = i;
                    now.v1 = tmp;
                    for(int i : myCommand.use)
                        tmp = i;
                    now.v2 = tmp;
                    move_list.push_back(now);
                }

                cout << "liveness: ";
                for (int var : myCommand.liveness)
                {
                    cout << "#" << var << ' ';
                    var_cnt = max(var_cnt, var);
                }
                cout << endl << endl;
            }
        }
        var_cnt ++;
        if(first)
        {
            for(int i = 0; i < var_cnt; ++i)
            {
                spilled.push_back(0);
                to.push_back(i);
            }
            first=false;
        }
        // cout<<var_cnt<<endl;
        interference_graph = new bool[var_cnt * var_cnt];
        degree = new int[var_cnt];
        removed = new bool[var_cnt];
        neighbors = new bool[var_cnt];
        var_rg = new int[var_cnt];
        for (int i = 0; i < var_cnt; ++ i)
            var_rg[i] = -1;  
        init();

        for(int block_id = 0; block_id < prog->block_num; block_id++) {
            
            for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {
                command myCommand = prog->program_block[block_id].program[com_id];
                for (int i : myCommand.liveness)
                    for (int j : myCommand.liveness)
                        if (i != j)
                        {
                            // cout<<i<<' '<<j<<endl;
                            interference_graph[i * var_cnt + j] = true;
                        }
            }
        }
        del_useless_move();
        int rg_cnt = 18;           
        rg_choices = new bool[rg_cnt];

        ok = allocate(rg_cnt);
        if (ok)
            break;
        else
        {
            bool possible = false;
            for(int vv = 0; vv<var_cnt; ++vv)
            {
                if(spilled[vv] == 0)
                {
                    possible=true;
                    break;
                }
            }
            if (!possible)
            {
                cout<<"The number of registers are not enough! Impossible to allocate!\n";
                return 0;
            }
            wrong.clear();
            for(int vv = 0; vv<var_cnt; ++vv)
            {
                to[vv]=vv;
                if(var_rg[vv]!=-1)
                    wrong.push_back(0);
                else
                {
                    wrong.push_back(1);
                    spilled[vv]=1;
                }
            }
            
            // delete []interference_graph;
            // interference_graph = NULL;
            // delete []degree;
            // degree = NULL;
            // delete []neighbors;
            // neighbors = NULL;
            // delete []var_rg;
            // var_rg = NULL;
            allocated_block.clear();
            tmp_prog = rewrite_program(prog);
            cout<<"\n";
            // delete prog;
            // prog = NULL;
            prog = tmp_prog;
        }
    }
    while(ok != true);

    for(int vv=0;vv<var_cnt;++vv)
        once_used_reg[find_reg_id(var_rg[vv])]=1;

    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num ; com_id++) {
            command *myCommand = &(prog->program_block[block_id].program[com_id]);
            get_op(myCommand);
            if(com_id == prog->program_block[block_id].com_num - 1 && com_id != 0)
                myCommand->func = JAL;
        }
    }
    
    // for(int block_id = 0; block_id < prog->block_num; block_id++) {
    //     cout<<"Block "<<block_id<<":"<<endl;
    //     for(int com_id = 0; com_id < prog->program_block[block_id].com_num - 1; com_id++) {
    //         command myCommand = prog->program_block[block_id].program[com_id]  //         cout<<myCommand.com_str<<" "<<opName[myCommand.func]<<endl;

    //         cout << "use: ";
    //         for (int var : myCommand.use)
    //         {
    //             cout << "#" << var << ' ';
    //         }
    //         cout<<endl;
    //         cout << "def: ";
    //         for (int var : myCommand.def)
    //         {
    //             cout << "#" << var << ' ';
    //         }
    //         cout << endl << endl;
    //     }
    // }

    caller_callee(prog);
    assemble_gen(prog);

    return 0;
}
