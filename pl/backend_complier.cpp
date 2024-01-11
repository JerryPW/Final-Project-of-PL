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
    bool checked = false;
};

struct Program
{
    vector<string> program;
    Block program_block[100];
    int block_num;
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


void check_block(struct Program* prog)
{
    for (int i = 0; i < prog->block_num; i++)
    {
        Block b = prog->program_block[i];
        cout << "Block: " << i << endl;
        for (int j = 0; j <= b.com_num; j++)
            cout << b.program[j].com_str << endl;
    }
}

bool check_func(string line)
{
    istringstream iss(line);
    string token;

    while (iss >> token) {
        if (token == "read_int()")
            return true;
    }
    return false;
}

int max_number(string line, int max)
{
    istringstream iss(line);
    string token;
    
    int var = -1;
    while (iss >> token) {
        if (token[0] == '#')
        {
            var = stoi(token.substr(1));
        }
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
                        var = stoi(token.substr(i + 1, j - i - 1));
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
                    var = stoi(token.substr(i + 1, j - i - 1));
                    i = j - 1;
                }
            }
        }
        
        if (var > max)
            max = var;
    }
    
    return max;
}


unordered_set <int> allocated_reg;
unordered_set <int> allocated_block;

void liveness_add(Program* prog, Block *block, command *pre_com, int now_block_id, int now_live_id, int now_var, int end_block_id, int end_com_id) {
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
    
    Block *now_block = &(prog->program_block[now_block_id]);
    command *now_com = &(now_block->program[now_com_id - 1]);
    command *pre_com  = new command;

    int end_com_id = now_com_id;
    int end_block_id = now_block_id;
    for (int now_var : now_com->liveness) {
        if (!allocated_reg.count(now_var)) {
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
        allocated_reg.insert(now_var);
    }
    // cout<<now_block_id<<" "<<now_com_id<<endl;
    if(now_com_id == 1 && now_block_id == 1)
        return;

    if(now_com_id > 1) {
        now_com_id--;
        liveness_gen(prog, now_block_id, now_com_id);
    } else {
        allocated_block.insert(now_block_id);
        for(int i = 0; i < now_block->parent.size();i++) {
            now_block_id = now_block->parent[i];
            now_com_id = prog->program_block[now_block_id].com_num;
            if(!allocated_block.count(now_block_id));
                liveness_gen(prog, now_block_id, now_com_id);
        }
    }
    
} 

vector<int> wrong;

struct Program* rewrite_program(struct Program* prog)
{
    struct Program* new_prog = new struct Program;
    
    int place = 16;
    for (int i = 0; i < prog->block_num; i++)
    {
        new_prog->program.push_back("Block " + to_string(i));
        struct Block temp_block = prog->program_block[i];
        for (int j = 0; j <=temp_block.com_num; j++)
        {
            struct command temp_com = temp_block.program[j];
            for (int var : temp_com.use)
            {
                if (wrong[var] && wrong[var] != 1)
                {
                    place = wrong[var];
                    new_prog->program.push_back("#" + to_string(var) + " = *(%rbp - " + to_string(place) + ")");
                }
            }
            new_prog->program.push_back(temp_com.com_str);
            for (int var : temp_com.def)
            {
                if (wrong[var] && wrong[var] == 1)
                {
                    new_prog->program.push_back("*(%rbp - " + to_string(place) + ") = #" + to_string(var));
                    wrong[var] = place;
                    place += 16;
                }
                else if(wrong[var])
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

int reg_id[]={6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

int * var_rg;
 

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
            if(!removed[i])
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
    for(i = 0; i < move_list.size(); ++ i)
    {
        if(move_list[i].coalesced)
        {
            if(var_rg[move_list[i].v1] != - 1)
                var_rg[move_list[i].v2] = var_rg[move_list[i].v1];
            else    
                var_rg[move_list[i].v1] = var_rg[move_list[i].v2];
        }
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
        if (token.find("read_int()") != std::string::npos)
            com->func = PSEUDO_JAL;
        else if (token.find("MUL") != std::string::npos)
            com->func = SLL;
        else if (token.find("PLUS") != std::string::npos)
            com->func = ADD;
        else if (token.find("MINUS") != std::string::npos)
            com->func = SUB;
        else if (token.find("if") != std::string::npos) {
            string condition;
            while (iss >> token && token != "then") {
                condition += token + " ";
            }
            size_t found = condition.find("GT");
            if (found != string::npos) 
                com->func = BGE;
            else {
                size_t found = condition.find("LT");
                if (found != string::npos) 
                    com->func = BLT;
            }
        } else if (token.find("%rbp") != std::string::npos) 
            com->func = LB;
        
    }
}

void assemble_gen_print(command *com) {
    if(com->func == PSEUDO_JAL) {
        int rd;
        for(int r : com->def)
            int rd = r;
        cout<<opName[com->func]<<endl;
        cout<<opName[0]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rd]<<", "<<"x1"<<endl;
    } else if (com->func == ADD) {
        auto rd = *(com->def.begin());
        if (com->use.size() == 0){
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x0"<<", "<<"#"<<com->com_str[com->com_str.find("=")+2]<<endl;
        }else if (com->use.size() == 1) {
            auto rs1 = *(com->use.begin());
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x0"<<endl;
        }else if (com->use.size() == 2) {
            auto rs1 = *(com->use.begin());
            int rs2;
            for(int r : com->use)
                int rs2 = r;
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x"<<var_rg[rs2]<<endl;
        }
    } else if (com->func == SUB) {
        auto rd = *(com->def.begin());
        if (com->use.size() == 1) {
            auto rs1 = *(com->use.begin());
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"#"<<com->com_str[com->com_str.find(", ")+2]<<endl;
        }else if (com->use.size() == 2) {
            auto rs1 = *(com->use.begin());
            int rs2;
            for(int r : com->use)
                int rs2 = r;
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x"<<var_rg[rs2]<<endl;
        }
    } else if (com->func == SLL) {
        auto rd = *(com->def.begin());
        auto rs1 = *(com->use.begin());
        int rs2;
        for(int r : com->use)
            int rs2 = r;
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<"x"<<var_rg[rs1]<<", "<<"x"<<var_rg[rs2]<<endl;
    } else if (com->func == LB) {
        if (com->use.size() == 0) {
            auto rd = *(com->def.begin());
            size_t startPos = com->com_str.find("%rbp - ");
            size_t endPos = com->com_str.find(")", startPos);
            cout<<opName[com->func]<<" "<<"x"<<var_rg[rd]<<", "<<com->com_str.substr(startPos, endPos - startPos)<<endl;
        } else if (com->def.size() == 0) {
            auto rd = *(com->use.begin());
            size_t startPos = com->com_str.find("%rbp - ");
            size_t endPos = com->com_str.find(")", startPos);
            cout<<opName[com->func]<<" "<<com->com_str.substr(startPos, endPos - startPos)<<", "<<"x"<<var_rg[rd]<<endl;
        }
    } else if (com->func == BGE) {
        auto rs = *(com->use.begin());
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
                size_t found = condition.find("GT");

                while (iss >> token && token != "else") {
                    thenOperation += token + " ";
                }

                while (iss >> token) {
                    elseOperation += token + " ";
                }
                break; 
            }
        }
        cout<<opName[com->func]<<" "<<"x"<<var_rg[rs]<<", "<<com->com_str[com->com_str.find(", ")+2]<<" "<<"Else"<<endl;
        size_t find = thenOperation.find("jmp ");
        char end = thenOperation[find+4];
        cout<<"jal Block "<<end<<endl;
        cout<<"Else:"<<endl;
        size_t find_two = elseOperation.find("jmp ");
        char end_two = elseOperation[find_two+4];
        cout<<"jal Block "<<end_two<<endl;
    } else if (com->func == JAL)
        cout<<opName[com->func]<<" Block "<<com->com_str[com->com_str.find("jmp ")+4]<<endl;
}

void assemble_gen(Program* prog) {
    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        cout<<"Block "<<block_id<<":"<<endl;
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num - 1; com_id++) {
            command *com = &(prog->program_block[block_id].program[com_id]);
            cout<<com->com_str<<": ";
            assemble_gen_print(com);
            // cout<<com->com_str<<" "<<opName[com->func]<<endl;
        }
    }
}

int main()
{
    Program *prog = new Program;
    Program *tmp_prog=NULL;
    ifstream ifs;
    ifs.open("output.txt", ios::in);

    int vars = -1;
    if (ifs.is_open())
    {
        string line;
        int i = 0;
        while (getline(ifs, line))
        {
            vars = max_number(line, vars);
            prog->program.push_back(line);
            i++;
        }
        prog->program.push_back("EOF");
    }
    else
        cout << "failed" << endl;
    int RAX_NUMBER = vars + 1;
    int i = 0;
    while (prog->program[i] != "EOF")
    {
        if (check_func(prog->program[i]))
        {
            string target = "read_int()";
            size_t pos = prog->program[i].find(target);
            prog->program[i].replace(pos, target.length(), "#" + to_string(RAX_NUMBER));
            string com = "#" + to_string(RAX_NUMBER) + " = read_int()";
            prog->program.insert(prog->program.begin() + i, com);
        }

        i++;
    }  
    int ok = false;
    do
    { 

        
        cout << "The RAX number is: " << RAX_NUMBER << endl;


        build_block(prog);
        check_block(prog);

        for(int block_id = 0; block_id < prog->block_num; block_id++) {
            for(int com_id = 0; com_id < prog->program_block[block_id].com_num - 1; com_id++) {
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
        //     for(int com_id = 0; com_id < prog->program_block[block_id].com_num; com_id++) {2
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
            // cout<<(prog->program_block[block_id].com_num - 1)<<endl;
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
        int rg_cnt = 4;
        rg_choices = new bool[rg_cnt];

        ok = allocate(rg_cnt);
        if (ok)
            break;
        else
        {
            wrong.clear();
            for(int vv = 0; vv<var_cnt; ++vv)
            {
                if(var_rg[vv]!=-1)
                    wrong.push_back(0);
                else
                    wrong.push_back(1);
            }
            
            // delete []interference_graph;
            // interference_graph = NULL;
            // delete []degree;
            // degree = NULL;
            // delete []neighbors;
            // neighbors = NULL;
            // delete []var_rg;
            // var_rg = NULL;
            allocated_reg.clear();
            allocated_block.clear();
            tmp_prog = rewrite_program(prog);
            cout<<"\n";
            // delete prog;
            // prog = NULL;
            prog = tmp_prog;
        }
    }
    while(ok != true);

    for(int block_id = 0; block_id < prog->block_num; block_id++) {
        for(int com_id = 0; com_id < prog->program_block[block_id].com_num - 1; com_id++) {
            command *myCommand = &(prog->program_block[block_id].program[com_id]);
            get_op(myCommand);
            if(com_id == prog->program_block[block_id].com_num - 2 && com_id != 0)
                myCommand->func = JAL;
        }
    }
    
    // for(int block_id = 0; block_id < prog->block_num; block_id++) {
    //     cout<<"Block "<<block_id<<":"<<endl;
    //     for(int com_id = 0; com_id < prog->program_block[block_id].com_num - 1; com_id++) {
    //         command myCommand = prog->program_block[block_id].program[com_id];
    //         cout<<myCommand.com_str<<" "<<opName[myCommand.func]<<endl;

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

    assemble_gen(prog);
    return 0;
}

