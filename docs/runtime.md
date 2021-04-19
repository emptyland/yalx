# Runtime



yalx_str *s

struct yalx_value_str {
    struct yalx_value_header header;
    uint32_t hash_code;
    uint32_t len;
    uint8_t bytes[0];
};

yalx_isolate_ops->enter_scope();
yalx_str *s = yalx_values_ops->new_string("ass");
if (!s) {
    goto end;
}
    return *s;
end:
yalx_isolate_ops->leave_scope();


typedef struct yalx_value_str * yalx_str;

if (yalx_str_ops->len(s)) {

}