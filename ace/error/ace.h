//this file is ace header4
#ifndef ACE
#define ACE

struct state_bit{
   uint32_t er : 3;
   uint32_t p1 : 3;
   uint32_t p2 : 3;
   uint32_t p3 : 3;
   uint32_t p4 : 3;
   uint32_t p5 : 3;
   uint32_t p6 : 3;
   uint32_t p7 : 3;
   uint32_t save : 1;
   uint32_t rb : 7;
};

void init_TLC_state(struct state_bit* states);
void init_MLC_state(struct state_bit* states);
float set_c_location(float std, float mean);
void read_retry(uint64_t* buf, float* voltage, struct state_bit* states, int retry_count,int ref);
int TLC_nand_sec_error(uint64_t* buf, int PE_cnt, uint64_t retention_time, int read_cnt, uint16_t* wear_out, uint64_t idx_wear_out, struct state_bit* states,float* voltage);
int MLC_nand_sec_error(uint64_t* buf, int PE_cnt, uint64_t retention_time, int read_cnt, uint16_t* wear_out, uint64_t idx_wear_out, struct state_bit* states,float* voltagea);

#define ace_err(fmt, ...) \
    do { fprintf(stderr, "[ACE] ACE-Err: " fmt, ## __VA_ARGS__); } while (0)

#define ace_log(fmt, ...) \
    do { printf("[ACE] ACE-Log: " fmt, ## __VA_ARGS__); } while (0)

/* ACE assert() */
#ifdef ACE_DEBUG
#define ace_assert(expression) assert(expression)
#else
#define ace_assert(expression)
#endif

#endif
