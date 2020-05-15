
#include <sbi/sbi_trap.h>

extern int decode_illegal(uint64_t, struct sbi_trap_regs*);

extern uint64_t mul_mulh(uint64_t, uint64_t);
extern uint64_t mul_mulhsu(uint64_t, uint64_t);
extern uint64_t mul_mulhu(uint64_t, uint64_t);
