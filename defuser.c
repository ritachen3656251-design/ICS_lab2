#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t a = (high * 5u) ^ (low + 0xb15eu);
    uint32_t b = (low * 5u + middle) ^ 0x1db3u;

    values[0] = (b + 4u * a) ^ 0xe881u;
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    values[0] = (uint32_t)((challenge >> 10) & 31u) + 7u;
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t part = (uint32_t)((challenge >> (i + 7)) & 15u);
        values[i] = (values[i - 1] ^ (part + i)) * 3u;
    }
    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);

    values[0] = index;
    switch (index) {
        case 0: values[1] = (x ^ 0x87ccu) + (x >> 2); break;
        case 1: values[1] = 5u * x + 0x49160u; break;
        case 2: values[1] = 5u * x + 0x9a24u; break;
        case 3: values[1] = (x ^ 0x4d93u) + (x >> 2); break;
        case 4: values[1] = 5u * x + 0xb829u; break;
        case 5: values[1] = 3u * x + 0xfe6au; break;
        case 6: values[1] = (4u * x) ^ 0x833du; break;
        case 7: values[1] = (2u * x) ^ 0x1105u; break;
    }
    return write_answer(out, cap, values, 2);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t salt) {
    uint32_t mid = low + (high - low) / 2u;
    uint32_t value;

    if (target == mid) {
        return (mid + 248u) ^ salt;
    }
    if (target < mid) {
        value = phase4_walk(target, low, mid - 1u, salt);
        return (3u * value) ^ (mid + salt) ^ 12u;
    }
    value = phase4_walk(target, mid + 1u, high, salt);
    return (3u * value) ^ (mid + salt) ^ 10u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)(challenge & 127u);
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(target, 0, 127, salt);
    return write_answer(out, cap, values, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    struct node {
        const struct node *next;
        uint32_t id;
        uint32_t bias;
    };
    static const struct node nodes[12] = {
        {&nodes[7], 2935, 57449},
        {&nodes[9], 841, 3920},
        {&nodes[6], 1313, 55132},
        {&nodes[1], 1555, 62310},
        {NULL, 280, 1036},
        {&nodes[8], 1049, 5848},
        {&nodes[4], 2427, 22857},
        {&nodes[10], 1918, 10615},
        {&nodes[3], 55, 37381},
        {&nodes[11], 2113, 61635},
        {&nodes[2], 2657, 51936},
        {&nodes[0], 529, 21879}
    };
    static const struct node *const heads[8] = {
        &nodes[5], &nodes[8], &nodes[3], &nodes[1],
        &nodes[9], &nodes[11], &nodes[0], &nodes[7]
    };
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t value = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    unsigned count = (unsigned)((challenge >> 3) & 3u) + 1u;
    const struct node *node = heads[challenge & 7u];

    value ^= 0x7581u;
    for (unsigned i = 0; i < count; ++i) {
        value = (3u * value + node->bias) ^ node->id;
        values[1] = node->id;
        node = node->next;
    }
    values[0] = value;
    return write_answer(out, cap, values, 2);
}

static uint32_t phase6_op0(uint32_t x) {
    return (x >> 1) | (x << 31);
}

static uint32_t phase6_op1(uint32_t x) {
    return (2u * x + 0xc3ecu) ^ (7u * x);
}

static uint32_t phase6_op2(uint32_t x) {
    return (x ^ (x >> 7)) + 0xe4a9u;
}

static uint32_t phase6_op3(uint32_t x) {
    return (x + 0xca25u) ^ (x >> 2);
}

static uint32_t phase6_op4(uint32_t x) {
    return ((x << 2) | (x >> 30)) ^ 0x62e5u;
}

static uint32_t phase6_op5(uint32_t x) {
    return (x ^ 0xf6bcu) + (x << 7);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    static uint32_t (*const ops[8])(uint32_t) = {
        phase6_op1, phase6_op5, phase6_op4, phase6_op0,
        phase6_op4, phase6_op3, phase6_op2, phase6_op3
    };
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    unsigned first = (unsigned)(challenge & 7u);
    unsigned second = (unsigned)((challenge >> 3) & 7u);

    x = ops[first](x);
    x ^= 0x8322u;
    x = ops[second](x);
    values[0] = x + 0x671cu;
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    static const char padding[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    uint32_t mode = (uint32_t)(context & 3u);
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t x = a ^ b;
    uint32_t first;
    uint32_t second;

    x = (x >> 5) | (x << 27);
    first = x ^ 0xf159u ^ (uint32_t)(context >> 16) ^ (mode * 0x6a9eu);
    second = first ^ (b + mode + 0x9bc7u + (a >> 5));

    int size = snprintf(out, cap, "%sOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        padding, mode, first, second);
    return size < 0 || (size_t)size >= cap;
}
