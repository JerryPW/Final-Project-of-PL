#define NONE 4294967295

unsigned int build_nat(char * c, int len);
char * new_str(char * str, int len);
struct SU_hash_table;
struct SU_hash_table * init_SU_hash();
unsigned int SU_hash_get(struct SU_hash_table * t, char * key);
void SU_hash_set(struct SU_hash_table * t, char * key, unsigned int value);
void SU_hash_delete(struct SU_hash_table * t, char * key);


