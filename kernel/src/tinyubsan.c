#include <stdint.h>

struct ubsan_source_location
{
    const char *file;
    uint32_t line;
    uint32_t column;
};

struct ubsan_type_descriptor
{
    uint16_t kind;
    uint16_t info;
    char name[];
};

struct ubsan_overflow_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_shift_out_of_bounds_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *left_type;
    struct ubsan_type_descriptor *right_type;
};

struct ubsan_invalid_value_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_array_out_of_bounds_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *array_type;
    struct ubsan_type_descriptor *index_type;
};

struct ubsan_type_mismatch_v1_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
    unsigned char log_alignment;
    unsigned char type_check_kind;
};

struct ubsan_negative_vla_data
{
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_nonnull_return_data
{
    struct ubsan_source_location location;
};

struct ubsan_nonnull_arg_data
{
    struct ubsan_source_location location;
};

struct ubsan_unreachable_data
{
    struct ubsan_source_location location;
};

struct ubsan_invalid_builtin_data
{
    struct ubsan_source_location location;
    unsigned char kind;
};

void ksp(char *fmt, ...);

#ifdef __cplusplus
extern "C"
{
#endif
    static void ubsan_ksp(const char *message, struct ubsan_source_location loc)
    {
        ksp("[ubsan][%s:%d:%d]: Triggered an Undefined Behaviour: %s\n", loc.file, loc.line, loc.column, message);
    }

    void __ubsan_handle_add_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Addition overflow", data->location);
    }

    void __ubsan_handle_sub_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Subtraction overflow", data->location);
    }

    void __ubsan_handle_mul_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Multiplication overflow", data->location);
    }

    void __ubsan_handle_divrem_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Division overflow", data->location);
    }

    void __ubsan_handle_negate_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Negation overflow", data->location);
    }

    void __ubsan_handle_pointer_overflow(struct ubsan_overflow_data *data)
    {
        ubsan_ksp("Pointer overflow", data->location);
    }

    void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_out_of_bounds_data *data)
    {
        ubsan_ksp("Shift out of bounds", data->location);
    }

    void __ubsan_handle_load_invalid_value(struct ubsan_invalid_value_data *data)
    {
        ubsan_ksp("Invalid load value", data->location);
    }

    void __ubsan_handle_out_of_bounds(struct ubsan_array_out_of_bounds_data *data)
    {
        ubsan_ksp("Array out of bounds", data->location);
    }

    void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch_v1_data *data, uintptr_t ptr)
    {
        if (!ptr)
        {
            ubsan_ksp("Use of NULL pointer", data->location);
        }

        else if (ptr & ((1UL << data->log_alignment) - 1))
        {
            ubsan_ksp("Use of misaligned pointer", data->location);
        }
        else
        {
            ubsan_ksp("No space for object", data->location);
        }
    }

    void __ubsan_handle_vla_bound_not_positive(struct ubsan_negative_vla_data *data)
    {
        ubsan_ksp("Variable-length argument is negative", data->location);
    }

    void __ubsan_handle_nonnull_return(struct ubsan_nonnull_return_data *data)
    {
        ubsan_ksp("Non-null return is null", data->location);
    }

    void __ubsan_handle_nonnull_arg(struct ubsan_nonnull_arg_data *data)
    {
        ubsan_ksp("Non-null argument is null", data->location);
    }

    void __ubsan_handle_builtin_unreachable(struct ubsan_unreachable_data *data)
    {

        ubsan_ksp("Unreachable code reached", data->location);
    }

    void __ubsan_handle_invalid_builtin(struct ubsan_invalid_builtin_data *data)
    {

        ubsan_ksp("Invalid builtin", data->location);
    }

#ifdef __cplusplus
}
#endif
