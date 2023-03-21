#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* 自己写的，很垃圾... */
static int lept_parse_literal(lept_context *c, lept_value *v, const char str) {
    EXPECT(c, str);
    switch (str) {
        case 'n':
            if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 3;
            v->type = LEPT_NULL;
            break;
        case 't':
            if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 3;
            v->type = LEPT_TRUE;
            break;
        case 'f':
            if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            c->json += 4;
            v->type = LEPT_FALSE;
            break;
        default:
            break;
    }
    return LEPT_PARSE_OK;
}

///
/// \param c 源 JSON 字符串
/// \param v JSON 结点
/// \param literal 需要检测的三种字面值（null true false）
/// \param type 判断的 JSON 结点类型
/// \return
static int lept_parse_literal_answer(lept_context *c, lept_value *v, const char *literal, lept_type type) {
    EXPECT(c, literal[0]);
    size_t i;
    for (i = 0; literal[i + 1]; i++) {
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    /* \TODO validate number */
    const char *p = c->json;
    if (*p == '-') // 首先检查是否以 - 开头，如果是直接跳过
        p++; // 以 - 开头只有两种情况：紧跟 0 或 1-9
    if (*p == '0') // 如果 0 紧跟在 - 后，直接跳过检查 0 后面那一项
        p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE; // 如果 - 后既不是 0 也不是 1-9，则值非法
        for (; ISDIGIT(*p); p++); // 如果 - 后是 1-9 开头，检查 1-9 后面，跳过所有数字
    }
    if (*p == '.') { // 遇到小数点，直接跳过，检查它后面一项是否是数字
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE; // 如果小数点后不是数字，则值非法
        for (; ISDIGIT(*p); p++); // 如果是，则跳过所有数字
    }
    if (*p == 'e' || *p == 'E') { // 遇到指数符号，跳过检查它后一位，它后一位允许有 - 或 +
        p++;
        if (*p == '-' || *p == '+') p++;
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE; // 指数位不是以 1-9 开始，则值非法
        for (; ISDIGIT(*p); p++); // 跳过指数位的所有数字
    }
    errno = 0;
    v->number = strtod(c->json, NULL); /* str 为要转换的字符串，end 为第一个不能转换的字符的指针*/
    /* 跳过前面的空白字符，直到遇上数字或正负符号才开始做转换，到出现非数字或字符串结束时('\0')才结束转换，并将结果返回 */
    if (errno == ERANGE && (v->number == HUGE_VAL || v->number == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case 't':
            return lept_parse_literal_answer(c, v, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal_answer(c, v, "false", LEPT_FALSE);
        case 'n':
            return lept_parse_literal_answer(c, v, "null", LEPT_NULL);
        default:
            return lept_parse_number(c, v);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        // 如果 parse 后后面还有字符
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->number;
}
