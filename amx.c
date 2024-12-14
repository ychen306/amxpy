#include "amx.h"

#define PTR_ROW_FLAGS(ptr, row, flags)                                         \
  (((uint64_t)&*(ptr)) + (((uint64_t)((row) + (flags) * 64)) << 56))

typedef union amx_reg {
  uint8_t u8[64];
  uint16_t u16[32];
  uint32_t u32[16];
  int8_t i8[64];
  int16_t i16[32];
  int32_t i32[16];
  _Float16 f16[32];
  float f32[16];
  double f64[8];
} amx_reg;

typedef __attribute__((aligned(128))) struct amx_state {
  amx_reg x[8];
  amx_reg y[8];
  amx_reg z[64];
} amx_state;

static void capture_state(amx_state *dst) {
  uint32_t row = 0;
  for (; row < 8; row += 2) {
    AMX_STX(PTR_ROW_FLAGS(dst->x[row].u8, row, 1));
    AMX_STY(PTR_ROW_FLAGS(dst->y[row].u8, row, 1));
    AMX_STZ(PTR_ROW_FLAGS(dst->z[row].u8, row, 1));
  }
  for (; row < 64; row += 2) {
    AMX_STZ(PTR_ROW_FLAGS(dst->z[row].u8, row, 1));
  }
}

static void inject_state(const amx_state *src) {
  uint32_t row = 0;
  for (; row < 8; row += 2) {
    AMX_LDX(PTR_ROW_FLAGS(src->x[row].u8, row, 1));
    AMX_LDY(PTR_ROW_FLAGS(src->y[row].u8, row, 1));
    AMX_LDZ(PTR_ROW_FLAGS(src->z[row].u8, row, 1));
  }
  for (; row < 64; row += 2) {
    AMX_LDZ(PTR_ROW_FLAGS(src->z[row].u8, row, 1));
  }
}

// Test bindings

typedef struct ldst_test_buffer {
  uint8_t bytes[256 + 128];
} ldst_test_buffer;

static uint64_t set_buf(uint64_t operand, uint8_t *buf, int interleave) {
  operand &= (0xffull << 56) | 0xff;
  if ((operand & (1ull << 62)) && interleave)
    operand &= ~0x7full;
  operand += (uint64_t)buf;
  return operand;
}

struct global_state {
  amx_state reg;
  ldst_test_buffer mem;
};

#define WRAP_LDST(OP)                                                          \
  struct global_state OP(uint64_t operand, struct global_state *in_state) {    \
    AMX_SET();                                                                 \
    struct global_state out_state = *in_state;                                 \
    inject_state(&out_state.reg);                                              \
    operand = set_buf(operand, out_state.mem.bytes, #OP[3] == 'I');            \
    AMX_##OP(operand);                                                         \
    capture_state(&out_state.reg);                                             \
    AMX_CLR();                                                                 \
    return out_state;                                                          \
  }

#define WRAP(OP)                                                               \
  struct global_state OP(uint64_t operand, struct global_state *in_state) {    \
    AMX_SET();                                                                 \
    struct global_state out_state = *in_state;                                 \
    AMX_##OP(operand);                                                         \
    capture_state(&out_state.reg);                                             \
    AMX_CLR();                                                                 \
    return out_state;                                                          \
  }

WRAP_LDST(LDX)
WRAP_LDST(LDY)
WRAP_LDST(STX)
WRAP_LDST(STY)
WRAP_LDST(LDZ)
WRAP_LDST(STZ)
WRAP_LDST(LDZI)
WRAP_LDST(STZI)
WRAP(EXTRX)
WRAP(EXTRY)
WRAP(MAC16)
WRAP(FMA16)
WRAP(FMA32)
WRAP(FMA64)
WRAP(FMS16)
WRAP(FMS32)
WRAP(FMS64)
WRAP(VECINT)
WRAP(VECFP)
WRAP(MATINT)
WRAP(MATFP)
WRAP(GENLUT)
