#include "md51.h"
#include <iomanip>
#include <assert.h>
#include <chrono>

using namespace std;
using namespace chrono;

Byte *StringProcess(string input, int *n_byte) {
    Byte *blocks = (Byte *)input.c_str();
    int length = input.length();

    int bitLength = length * 8;
    int paddingBits = bitLength % 512;
    if (paddingBits > 448) {
        paddingBits = 512 - (paddingBits - 448);
    }
    else if (paddingBits < 448) {
        paddingBits = 448 - paddingBits;
    }
    else if (paddingBits == 448) {
        paddingBits = 512;
    }

    int paddingBytes = paddingBits / 8;
    int paddedLength = length + paddingBytes + 8;
    Byte *paddedMessage = new Byte[paddedLength];

    memcpy(paddedMessage, blocks, length);
    paddedMessage[length] = 0x80;
    memset(paddedMessage + length + 1, 0, paddingBytes - 1);

    for (int i = 0; i < 8; ++i) {
        paddedMessage[length + paddingBytes + i] = ((uint64_t)length * 8 >> (i * 8)) & 0xFF;
    }

    *n_byte = paddedLength;
    return paddedMessage;
}

void MD5Hash_NEON(string inputs[4], bit32 states[4][4]) {
    Byte *paddedMessages[4];
    int messageLengths[4];
    
    for (int i = 0; i < 4; i++) {
        paddedMessages[i] = StringProcess(inputs[i], &messageLengths[i]);
    }
    
    uint32x4_t state0 = vdupq_n_u32(0x67452301);
    uint32x4_t state1 = vdupq_n_u32(0xefcdab89);
    uint32x4_t state2 = vdupq_n_u32(0x98badcfe);
    uint32x4_t state3 = vdupq_n_u32(0x10325476);
    
    for (int block = 0; block < messageLengths[0] / 64; block++) {
        uint32x4_t x[16];
        
        for (int i = 0; i < 16; i++) {
            uint32_t words[4];
            for (int j = 0; j < 4; j++) {
                words[j] = (paddedMessages[j][4 * i + block * 64]) |
                          (paddedMessages[j][4 * i + 1 + block * 64] << 8) |
                          (paddedMessages[j][4 * i + 2 + block * 64] << 16) |
                          (paddedMessages[j][4 * i + 3 + block * 64] << 24);
            }
            x[i] = vld1q_u32(words);
        }
        
        uint32x4_t a = state0, b = state1, c = state2, d = state3;
        
        /* Round 1 */
        FF(a, b, c, d, x[0][0], s11, 0xd76aa478);
        FF(d, a, b, c, x[1][0], s12, 0xe8c7b756);
        FF(c, d, a, b, x[2][0], s13, 0x242070db);
        FF(b, c, d, a, x[3][0], s14, 0xc1bdceee);
        FF(a, b, c, d, x[4][0], s11, 0xf57c0faf);
        FF(d, a, b, c, x[5][0], s12, 0x4787c62a);
        FF(c, d, a, b, x[6][0], s13, 0xa8304613);
        FF(b, c, d, a, x[7][0], s14, 0xfd469501);
        FF(a, b, c, d, x[8][0], s11, 0x698098d8);
        FF(d, a, b, c, x[9][0], s12, 0x8b44f7af);
        FF(c, d, a, b, x[10][0], s13, 0xffff5bb1);
        FF(b, c, d, a, x[11][0], s14, 0x895cd7be);
        FF(a, b, c, d, x[12][0], s11, 0x6b901122);
        FF(d, a, b, c, x[13][0], s12, 0xfd987193);
        FF(c, d, a, b, x[14][0], s13, 0xa679438e);
        FF(b, c, d, a, x[15][0], s14, 0x49b40821);
        
        /* Round 2 */
        GG(a, b, c, d, x[1][0], s21, 0xf61e2562);
        GG(d, a, b, c, x[6][0], s22, 0xc040b340);
        GG(c, d, a, b, x[11][0], s23, 0x265e5a51);
        GG(b, c, d, a, x[0][0], s24, 0xe9b6c7aa);
        GG(a, b, c, d, x[5][0], s21, 0xd62f105d);
        GG(d, a, b, c, x[10][0], s22, 0x2441453);
        GG(c, d, a, b, x[15][0], s23, 0xd8a1e681);
        GG(b, c, d, a, x[4][0], s24, 0xe7d3fbc8);
        GG(a, b, c, d, x[9][0], s21, 0x21e1cde6);
        GG(d, a, b, c, x[14][0], s22, 0xc33707d6);
        GG(c, d, a, b, x[3][0], s23, 0xf4d50d87);
        GG(b, c, d, a, x[8][0], s24, 0x455a14ed);
        GG(a, b, c, d, x[13][0], s21, 0xa9e3e905);
        GG(d, a, b, c, x[2][0], s22, 0xfcefa3f8);
        GG(c, d, a, b, x[7][0], s23, 0x676f02d9);
        GG(b, c, d, a, x[12][0], s24, 0x8d2a4c8a);
        
        /* Round 3 */
        HH(a, b, c, d, x[5][0], s31, 0xfffa3942);
        HH(d, a, b, c, x[8][0], s32, 0x8771f681);
        HH(c, d, a, b, x[11][0], s33, 0x6d9d6122);
        HH(b, c, d, a, x[14][0], s34, 0xfde5380c);
        HH(a, b, c, d, x[1][0], s31, 0xa4beea44);
        HH(d, a, b, c, x[4][0], s32, 0x4bdecfa9);
        HH(c, d, a, b, x[7][0], s33, 0xf6bb4b60);
        HH(b, c, d, a, x[10][0], s34, 0xbebfbc70);
        HH(a, b, c, d, x[13][0], s31, 0x289b7ec6);
        HH(d, a, b, c, x[0][0], s32, 0xeaa127fa);
        HH(c, d, a, b, x[3][0], s33, 0xd4ef3085);
        HH(b, c, d, a, x[6][0], s34, 0x4881d05);
        HH(a, b, c, d, x[9][0], s31, 0xd9d4d039);
        HH(d, a, b, c, x[12][0], s32, 0xe6db99e5);
        HH(c, d, a, b, x[15][0], s33, 0x1fa27cf8);
        HH(b, c, d, a, x[2][0], s34, 0xc4ac5665);
        
        /* Round 4 */
        II(a, b, c, d, x[0][0], s41, 0xf4292244);
        II(d, a, b, c, x[7][0], s42, 0x432aff97);
        II(c, d, a, b, x[14][0], s43, 0xab9423a7);
        II(b, c, d, a, x[5][0], s44, 0xfc93a039);
        II(a, b, c, d, x[12][0], s41, 0x655b59c3);
        II(d, a, b, c, x[3][0], s42, 0x8f0ccc92);
        II(c, d, a, b, x[10][0], s43, 0xffeff47d);
        II(b, c, d, a, x[1][0], s44, 0x85845dd1);
        II(a, b, c, d, x[8][0], s41, 0x6fa87e4f);
        II(d, a, b, c, x[15][0], s42, 0xfe2ce6e0);
        II(c, d, a, b, x[6][0], s43, 0xa3014314);
        II(b, c, d, a, x[13][0], s44, 0x4e0811a1);
        II(a, b, c, d, x[4][0], s41, 0xf7537e82);
        II(d, a, b, c, x[11][0], s42, 0xbd3af235);
        II(c, d, a, b, x[2][0], s43, 0x2ad7d2bb);
        II(b, c, d, a, x[9][0], s44, 0xeb86d391);
        
        state0 = vaddq_u32(state0, a);
        state1 = vaddq_u32(state1, b);
        state2 = vaddq_u32(state2, c);
        state3 = vaddq_u32(state3, d);
    }
    
    uint32_t result0[4], result1[4], result2[4], result3[4];
    vst1q_u32(result0, state0);
    vst1q_u32(result1, state1);
    vst1q_u32(result2, state2);
    vst1q_u32(result3, state3);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            uint32_t value;
            switch (i) {
                case 0: value = result0[j]; break;
                case 1: value = result1[j]; break;
                case 2: value = result2[j]; break;
                case 3: value = result3[j]; break;
            }
            states[j][i] = ((value & 0xff) << 24) |
                           ((value & 0xff00) << 8) |
                           ((value & 0xff0000) >> 8) |
                           ((value & 0xff000000) >> 24);
        }
    }
    
    for (int i = 0; i < 4; i++) {
        delete[] paddedMessages[i];
    }
}
