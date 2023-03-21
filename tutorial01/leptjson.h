#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/* JSON 是一个树形结构，最终需要实现一个树的数据结构，每个节点使用 lept_value 结构体表示 */
typedef struct {
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE, /* 一个 JSON 只含有空白 */
    LEPT_PARSE_INVALID_VALUE, /* 不是那三种字面值 (null false true) */
    LEPT_PARSE_ROOT_NOT_SINGULAR /* 一个值之后，在空白之后还有其他字符 */
};

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);

#endif /* LEPTJSON_H__ */
