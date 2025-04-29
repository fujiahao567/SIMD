#include <iostream>
#include <string>
#include <cstring>
#include <arm_neon.h>

using namespace std;

typedef unsigned char Byte;
typedef unsigned int bit32;

#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

// NEON优化的MD5基本函数
static inline uint32x4_t F(uint32x4_t x, uint32x4_t y, uint32x4_t z) {
    return vorrq_u32(vandq_u32(x, y), vandq_u32(vmvnq_u32(x), z));
}

static inline uint32x4_t G(uint32x4_t x, uint32x4_t y, uint32x4_t z) {
    return vorrq_u32(vandq_u32(x, z), vandq_u32(y, vmvnq_u32(z)));
}

static inline uint32x4_t H(uint32x4_t x, uint32x4_t y, uint32x4_t z) {
    return veorq_u32(veorq_u32(x, y), z);
}

static inline uint32x4_t I(uint32x4_t x, uint32x4_t y, uint32x4_t z) {
    return veorq_u32(y, vorrq_u32(x, vmvnq_u32(z)));
}

static inline uint32x4_t ROTATELEFT(uint32x4_t num, int n) {
    return vorrq_u32(vshlq_n_u32(num, n), vshrq_n_u32(num, 32-(n)));
}

// 修改宏定义，确保类型一致
#define FF(a, b, c, d, x_val, s, ac_val) { \
    uint32x4_t x_vec = vdupq_n_u32(x_val); \
    uint32x4_t ac_vec = vdupq_n_u32(ac_val); \
    a = vaddq_u32(a, F(b, c, d)); \
    a = vaddq_u32(a, x_vec); \
    a = vaddq_u32(a, ac_vec); \
    a = ROTATELEFT(a, s); \
    a = vaddq_u32(a, b); \
}

#define GG(a, b, c, d, x_val, s, ac_val) { \
    uint32x4_t x_vec = vdupq_n_u32(x_val); \
    uint32x4_t ac_vec = vdupq_n_u32(ac_val); \
    a = vaddq_u32(a, G(b, c, d)); \
    a = vaddq_u32(a, x_vec); \
    a = vaddq_u32(a, ac_vec); \
    a = ROTATELEFT(a, s); \
    a = vaddq_u32(a, b); \
}

#define HH(a, b, c, d, x_val, s, ac_val) { \
    uint32x4_t x_vec = vdupq_n_u32(x_val); \
    uint32x4_t ac_vec = vdupq_n_u32(ac_val); \
    a = vaddq_u32(a, H(b, c, d)); \
    a = vaddq_u32(a, x_vec); \
    a = vaddq_u32(a, ac_vec); \
    a = ROTATELEFT(a, s); \
    a = vaddq_u32(a, b); \
}

#define II(a, b, c, d, x_val, s, ac_val) { \
    uint32x4_t x_vec = vdupq_n_u32(x_val); \
    uint32x4_t ac_vec = vdupq_n_u32(ac_val); \
    a = vaddq_u32(a, I(b, c, d)); \
    a = vaddq_u32(a, x_vec); \
    a = vaddq_u32(a, ac_vec); \
    a = ROTATELEFT(a, s); \
    a = vaddq_u32(a, b); \
}

void MD5Hash(string input, bit32 *state);
void MD5Hash_NEON(string inputs[4], bit32 states[4][4]);
