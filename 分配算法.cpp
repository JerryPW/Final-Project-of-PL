#include<cstdio>
using namespace std;

/*
变量采用数字存储
需要把初始程序的变量修改为数字
例如，x = x + 1 -> (0) = (0) + 1
*/

int var_cnt = 0;

struct liveness
{
    int size;
    int *vars = new int[size];
    liveness *next;
};

liveness *liveness_list;

struct move_command
{
    int v1, v2;
    bool coalesced, invalid;
    move_command *next;
};

move_command *move_list;

bool *interference_graph = new bool[var_cnt * var_cnt];

void init()
{
    int i, j;
    for (i = 0; i < var_cnt; ++ i)
        for (j = 0; j < var_cnt; ++ j)
            interference_graph[i * var_cnt + j] = false;
}

void build_graph()
{
    liveness *now = liveness_list;

    int i, j;

    while (now != NULL)
    {
        for (i = 0; i < now->size; ++ i)
            for (j = i + 1; j < now->size; ++ j)
                interference_graph[now->vars[i] * var_cnt + now->vars[j]] = true;
                interference_graph[now->vars[j] * var_cnt + now->vars[i]] = true;
        now = now->next;
    }
}

void del_useless_move()
{
    move_command *now = move_list;
    while (now != NULL)
    {
        if (interference_graph[now->v1 * var_cnt + now->v2])
            now->invalid = true;
        now = now->next;
    }
}

int *degree = new int[var_cnt];

bool *removed = new bool[var_cnt];

void get_degrees()
{
    int i, j;
    for (i = 0; i < var_cnt; ++i)
    {
        degree[i] = 0;
        if(!removed[i])
            for (j = 0; j < var_cnt; ++j)
                if(!removed[i] )//j没有被删除
                    if (!removed[j] && i != j && interference_graph[i * var_cnt + j])
                        ++ degree[i];
    }
}

void get_move_info()
{
    int tmp;
    move_command *now = move_list;
    while (now != NULL)
    {
        if (now->coalesced || now->invalid)
            continue;
        tmp = now->v1;
        interference_graph[tmp * var_cnt + tmp] = true;
        tmp = now->v2;
        interference_graph[tmp * var_cnt + tmp] = true;
        now = now->next;
    }
}

//用于判断公共邻居
bool *neighbors = new bool[var_cnt];

void init_neighbors()
{
    int i;
    for (i = 0; i < var_cnt; ++ i)
        neighbors[i] = false;
}

bool allocate(int rg_cnt)
{
    struct remove_stk
    {
        int id;
        bool rm_type; // false for simplify, true for spill.
        remove_stk *next;
    };
    remove_stk *rm_stk = NULL;

    int res_cnt = var_cnt, i, deg_cnt, tmp;
    for (i = 0; i < var_cnt; ++ i)
        removed[i] = false;

    int * var_rg = new int[var_cnt];
    for (i = 0; i < var_cnt; ++ i)
        var_rg[i] = -1;   

    bool flag = false;

    //寄存器分配算法
    while(res_cnt)
    {
        //首先检查能否simplify
        flag = false;
        get_degrees();
        get_move_info();
        for (i = 0; i < var_cnt; ++ i)
        {
            if (!removed[i] && !interference_graph[i * var_cnt + i] && degree[i] < rg_cnt - 1)
            {
                removed[i] = true;
                -- res_cnt;
                remove_stk *now = new remove_stk;
                now->id = i;
                now->rm_type = false;
                now->next = rm_stk;
                rm_stk = now;
                
                flag = true;
                break;
            }
        }
        if(flag)
            continue;
        
        //保守合并
        move_command *now = move_list;
        
        while (now != NULL)
        {
            if (now->coalesced || now->invalid)
            {
                now = now->next;
                continue;
            }
            init_neighbors();
            for (i = 0; i < var_cnt; ++ i)
            {
                if(!removed[i] && i != now->v1 && i != now->v2)
                    if(interference_graph[i * var_cnt + now->v1] && interference_graph[i * var_cnt + now->v2])
                        neighbors[i] = true;
            }
            //策略1
            deg_cnt = 0;
            for (i = 0; i < var_cnt; ++ i)
                if(neighbors[i] && degree[i] >= rg_cnt)
                    ++deg_cnt;
            if (deg_cnt < rg_cnt)
            {
                removed[now->v2] = true;
                now->coalesced = true;
                -- res_cnt;
                flag = true;
                break;
            }
            //策略2
            for (i = 0; i < var_cnt; ++ i)
                if(!removed[i] && i != now->v1 && interference_graph[i * var_cnt + now->v1])
                    if(!neighbors[i] && degree[i] >= rg_cnt)
                    {
                        flag = true;
                        break;   
                    }
            if (!flag)
            {
                removed[now->v1] = true;
                now->coalesced = true;
                -- res_cnt;
                flag = true;
                break;             
            }

            for (i = 0; i < var_cnt; ++ i)
                if(!removed[i] && i != now->v2 && interference_graph[i * var_cnt + now->v2])
                    if(!neighbors[i] && degree[i] >= rg_cnt)
                    {
                        flag = true;
                        break;   
                    }
            if (!flag)
            {
                removed[now->v2] = true;
                now->coalesced = true;
                -- res_cnt;
                flag = true;
                break;          
            }
            now = now->next;
            flag = false;
        }

        if(flag)//有合并行为
            continue;

        //Freeze
        now = move_list;
        while (now != NULL)
        {
            if (now->coalesced || now->invalid)
            {
                now = now->next;
                continue;
            }
            now->invalid = true;
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
                removed[i] = true;
                -- res_cnt;
                remove_stk *now = new remove_stk;
                now->id = i;
                now->rm_type = true;
                now->next = rm_stk;
                rm_stk = now;                
            }
        }
    }
    
    //开始分配寄存器
    bool *rg_choices = new bool[rg_cnt];
    flag = false;

    remove_stk *cur = rm_stk;
    while(cur != NULL)
    {        
        tmp = cur->id;
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

        cur = cur->next;
    }
    //根据var_rg的信息即可知道哪些变量是真spill
    if(flag)
        return false;
    else
        return true;
}

int main()
{
    return 0;
}