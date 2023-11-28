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
    bool  coalesced;
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
        tmp = now->v1;
        interference_graph[tmp * var_cnt + tmp] = true;
        tmp = now->v2;
        interference_graph[tmp * var_cnt + tmp] = true;
        now = now->next;
    }
}

void allocate(int rg_cnt)
{
    struct remove_stk
    {
        int id;
        bool rm_type; // false for simplify, true for spill.
        remove_stk *next;
    };
    remove_stk *rm_stk = NULL;

    int res_cnt = var_cnt, i;
    for (i = 0; i < var_cnt; ++ i)
        removed[i] = false;

    bool flag = false;
    while(res_cnt)
    {
        //首先检查能否simplify
        flag = false;
        get_degrees();
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
            if (now -> coalesced)
            {
                now = now->next;
                continue;
            }
            //策略1
            
            //策略2

        }
    }
}

int main()
{
    return 0;
}