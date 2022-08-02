#include "types.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdalign.h>

#define STRINGFY(x)     #x
#define STRINGFY2(x)    STRINGFY(x)

#define FLAG_TYPE       u8
#define LITERAL_TYPE    char*
#define STRING_TYPE     char*
#define UINT_TYPE       u64
#define INT_TYPE        i64
#define FLOAT_TYPE      f64
// #define FLAG_TYPE       unsigned char
// #define LITERAL_TYPE    char*
// #define STRING_TYPE     char*
// #define UINT_TYPE       unsigned long long
// #define INT_TYPE        long long
// #define FLOAT_TYPE      double

#define FLAG_TYPENAME       STRINGFY2(FLAG_TYPE)
#define LITERAL_TYPENAME    STRINGFY2(LITERAL_TYPE)
#define STRING_TYPENAME     STRINGFY2(STRING_TYPE)
#define UINT_TYPENAME       STRINGFY2(UINT_TYPE)
#define INT_TYPENAME        STRINGFY2(INT_TYPE)
#define FLOAT_TYPENAME      STRINGFY2(FLOAT_TYPE)

#define FLAG_SIZE       sizeof(FLAG_TYPE)
#define LITERAL_SIZE    sizeof(LITERAL_TYPE)
#define STRING_SIZE     sizeof(STRING_TYPE)
#define UINT_SIZE       sizeof(UINT_TYPE)
#define INT_SIZE        sizeof(INT_TYPE)
#define FLOAT_SIZE      sizeof(FLOAT_TYPE)

#define FLAG_ALIGN      alignof(FLAG_TYPE)
#define LITERAL_ALIGN   alignof(LITERAL_TYPE)
#define STRING_ALIGN    alignof(STRING_TYPE)
#define UINT_ALIGN      alignof(UINT_TYPE)
#define INT_ALIGN       alignof(INT_TYPE)
#define FLOAT_ALIGN     alignof(FLOAT_TYPE)

#define GET_OFFSET(T, field)            ((u64) &(((T *)0)->field))
#define GET_OFFSET_END(T, fstfield)     ((u64) &((((T *)0)+1)->fstfield))

typedef enum {
    ARGS_flag, ARGS_literal, ARGS_string,
    ARGS_uint, ARGS_int, ARGS_float
} ARGS_ArgType;

typedef struct {
    char *small;
    char *big;
    char *description;
    ARGS_ArgType type;
    u32 ret_offset;
    b32 found;
} ARGS_ArgOption;

typedef enum {
    ARGS_SPECFLAGS_no_implicit_help  = 0x01,
    ARGS_SPECFLAGS_unused_02         = 0x02,
    ARGS_SPECFLAGS_unused_04         = 0x04,
    ARGS_SPECFLAGS_unused_08         = 0x08,
    ARGS_SPECFLAGS_unused_10         = 0x10,
    ARGS_SPECFLAGS_unused_20         = 0x20,
    ARGS_SPECFLAGS_unused_40         = 0x40,
    ARGS_SPECFLAGS_log               = 0x80,
} ARGS_SpecFlags;

typedef struct {
    u32 flags;
    u32 options_len;
    ARGS_ArgOption *options;
} ARGS_ArgSpec;

const char* ARGS_typename(const ARGS_ArgType type) {
    switch ( type ) {
        case ARGS_flag:
            return FLAG_TYPENAME;
        case ARGS_literal:
            return LITERAL_TYPENAME;
        case ARGS_string:
            return STRING_TYPENAME;
        case ARGS_uint:
            return UINT_TYPENAME;
        case ARGS_int:
            return INT_TYPENAME;
        case ARGS_float:
            return FLOAT_TYPENAME;
        default:
            assert( 0 && "unreachable" );
            break;
    }
}

size_t ARGS_sizeof(const ARGS_ArgType type) {
    switch ( type ) {
        case ARGS_flag:
            return FLAG_SIZE;
        case ARGS_literal:
            return LITERAL_SIZE;
        case ARGS_string:
            return STRING_SIZE;
        case ARGS_uint:
            return UINT_SIZE;
        case ARGS_int:
            return INT_SIZE;
        case ARGS_float:
            return FLOAT_SIZE;
        default:
            assert( 0 && "unreachable" );
            break;
    }
}

size_t ARGS_alignof(const ARGS_ArgType type) {
    switch ( type ) {
        case ARGS_flag:
            return FLAG_ALIGN;
        case ARGS_literal:
            return LITERAL_ALIGN;
        case ARGS_string:
            return STRING_ALIGN;
        case ARGS_uint:
            return UINT_ALIGN;
        case ARGS_int:
            return INT_ALIGN;
        case ARGS_float:
            return FLOAT_ALIGN;
        default:
            assert( 0 && "unreachable" );
            break;
    }
}

void ARGS_log(const ARGS_ArgSpec spec, const char * format, ... ) {
    if ( spec.flags & ARGS_SPECFLAGS_log ) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void ARGS_implicit_help(const ARGS_ArgSpec spec,
        const char *prog_name) {
    assert( !(spec.flags & ARGS_SPECFLAGS_no_implicit_help) );
    printf("usage: %s [flags]\n\n", prog_name);
    printf("Accepted flags:\n");
    for ( size_t i = 0; i < spec.options_len; i++ ) {
        const ARGS_ArgOption option = spec.options[i];
        assert( option.small );
        assert( option.big );
        assert( option.small[0] || option.big[0] );
        printf("  ");
        if ( option.small[0] ) {
            printf("-%c%c ", option.small[0],
                    option.big[0] ? ',' : ' ');
        } else {
            printf("    ");
        }
        if ( option.big[0] ) {
            printf("--%s ", option.big);
        } else {
            printf(" ");
        }
        switch ( option.type ) {
            case ARGS_flag:
                printf("\n");
                break;
            case ARGS_literal:
                printf("<text_literal>\n");
                break;
            case ARGS_string:
                printf("<string>\n");
                break;
            case ARGS_uint:
                printf("<unsigned_integer>\n");
                break;
            case ARGS_int:
                printf("<signed_integer>\n");
                break;
            case ARGS_float:
                printf("<floating_number>\n");
                break;
            default:
                assert( 0 && "unreachable" );
                break;
        }
        if ( option.description && option.description[0] ) {
            const char *desc = option.description;
            int start = 0, end = 0;
            for ( ; desc[end]; end++ ) {
                if ( desc[end] == '\n' ) {
                    printf("     %.*s\n", end - start, desc + start);
                    start = end + 1;
                }
            }
            printf("     %.*s\n\n", end - start, desc + start);
        } else {
            printf("\n");
        }
    }
}

inline static
size_t ARGS_align_to(size_t size, size_t this_align) {
    return size & (- this_align);
}

size_t ARGS_calc_retsize_and_cache(ARGS_ArgSpec spec,
        size_t *valid_offset) {
    size_t size = 0;
    size_t max_align = 1;
    for ( u64 i = 0; i < spec.options_len; i++ ) {
        ARGS_ArgOption *this = spec.options + i;
        const size_t this_size = ARGS_sizeof(this->type);
        const size_t this_align = ARGS_alignof(this->type);
        const size_t padd = size - ARGS_align_to(size, this_align);
        this->ret_offset = size + padd;
        size += padd + this_size;
        max_align = this_align > max_align ? this_align : max_align;
    }
    {
        // ARGS_valid
        const size_t valid_size = ARGS_sizeof(ARGS_flag);
        const size_t valid_align = ARGS_alignof(ARGS_flag);
        const size_t padd = size - ARGS_align_to(size, valid_align);
        *valid_offset = size + padd;
        size += padd + valid_size;
    }
    const size_t fsize = ARGS_align_to(size, max_align) + max_align;
    assert( (fsize / max_align) * max_align == fsize );
    return fsize;
}

b32 ARGS_streq(const char *a, const char *b) {
    for ( size_t i = 0; a[i] && b[i]; i++ ) {
        if ( a[i] != b[i] ) return 0;
    }
    return 1;
}

size_t ARGS_find_index_small(const ARGS_ArgSpec spec, const char *arg) {
    size_t index = spec.options_len + 1;
    if ( !(spec.flags & ARGS_SPECFLAGS_no_implicit_help) ) {
        if ( ARGS_streq(arg, "h") ) {
            index = spec.options_len;
        }
    }
    for ( size_t i = 0; i < spec.options_len; i++ ) {
        const ARGS_ArgOption option = spec.options[i];
        if ( option.small[0] && ARGS_streq(arg, option.small) ) {
            assert( index >= spec.options_len );
            index = i;
        }
    }
    return index;
}

size_t ARGS_find_index_big(const ARGS_ArgSpec spec, const char *arg) {
    size_t index = spec.options_len + 1;
    if ( !(spec.flags & ARGS_SPECFLAGS_no_implicit_help) ) {
        if ( ARGS_streq(arg, "h") ) {
            index = spec.options_len;
        }
    }
    for ( size_t i = 0; i < spec.options_len; i++ ) {
        const ARGS_ArgOption option = spec.options[i];
        if ( option.big[0] && ARGS_streq(arg, option.big) ) {
            assert( index >= spec.options_len );
            index = i;
        }
    }
    return index;
}

size_t ARGS_find_index(const ARGS_ArgSpec spec, const char *arg) {
    size_t index = spec.options_len + 1;
    if ( arg[0] != '-' || arg[1] == '\0' ) {
        // Nothing to do
    } else if ( arg[1] == '-' ) {
        assert( arg[2] );
        index = ARGS_find_index_big(spec, arg + 2);
    } else if ( arg[1] ) {
        index = ARGS_find_index_small(spec, arg + 1);
    }
    return index;
}

FLAG_TYPE ARGS_parse_flag(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    (void) ip; (void) argc; (void) argv; (void) spec;
    return 1;
}

LITERAL_TYPE ARGS_parse_literal(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    (void) ip; (void) argc; (void) argv; (void) spec;
    assert( 0 && "ARGS_parse_literal: not implemented" );
    return 0;
}

STRING_TYPE ARGS_parse_string(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    (void) ip; (void) argc; (void) argv; (void) spec;
    assert( 0 && "ARGS_parse_string: not implemented" );
    return 0;
}

UINT_TYPE ARGS_parse_uint(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    *ip += 1;
    assert( *ip < argc );
    UINT_TYPE ret = 0;
    for ( const char *arg = argv[*ip]; *arg; arg++ ) {
        const char c = *arg;
        UINT_TYPE new_ret = 0;
        if ( '0' <= c && c <= '9' ) {
            new_ret = ret * 10 + c - '0';
        } else {
            ARGS_log(spec,
                    "Warning truncating invalid uint at %d `%s'",
                    *ip, argv[*ip]);
            break;
        }
        assert( ret <= new_ret );
        ret = new_ret;
    }
    return ret;
}

INT_TYPE ARGS_parse_int(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    (void) ip; (void) argc; (void) argv; (void) spec;
    assert( 0 && "ARGS_parse_int: not implemented" );
    return 0;
}

FLOAT_TYPE ARGS_parse_float(int *ip,
        const int argc, const char **argv,
        const ARGS_ArgSpec spec) {
    *ip += 1;
    assert( *ip < argc );
    FLOAT_TYPE ret = 0;
    const char *arg = argv[*ip];
    // TODO: add support to sign
    for ( ; *arg; arg++ ) {
        const char c = *arg;
        if ( c == '.' ) break;

        FLOAT_TYPE new_ret = 0;
        if ( '0' <= c && c <= '9' ) {
            new_ret = ret * 10 + ((FLOAT_TYPE) (c - '0'));
        } else {
            ARGS_log(spec,
                    "Warning truncating invalid float at %d `%s'",
                    *ip, argv[*ip]);
            break;
        }
        assert( ret <= new_ret );
        ret = new_ret;
    }
    if ( arg[0] == '.' ) {
        arg += 1;
        for ( FLOAT_TYPE f = 0.1; *arg; arg++, f *= 0.1 ) {
            const char c = *arg;
            if ( c == '.' ) break;

            FLOAT_TYPE new_ret = 0;
            if ( '0' <= c && c <= '9' ) {
                new_ret = ret + ((FLOAT_TYPE) (c - '0')) * f;
            } else {
                ARGS_log(spec,
                        "Warning truncating invalid float at %d `%s'",
                        *ip, argv[*ip]);
                break;
            }
            assert( ret <= new_ret );
            ret = new_ret;
        }
    }
    return ret;
}

void* ARGS_parse(ARGS_ArgSpec spec, void *ret, size_t size,
        const int argc, const char **argv) {
    assert( ret );
    size_t valid_offset = 0;
    const size_t retsize =
        ARGS_calc_retsize_and_cache(spec, &valid_offset);
    assert( size == retsize && "sizes got calculated right" );
    b32 is_valid = 1;
    for ( int i = 1; i < argc; i++ ) {
        const size_t opt_index = ARGS_find_index(spec, argv[i]);
        if ( opt_index < spec.options_len ) {
            const ARGS_ArgOption option = spec.options[opt_index];
            const u32 offset = option.ret_offset;
            switch ( option.type ) {
                case ARGS_flag: {
                    FLAG_TYPE *p =
                        (FLAG_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_flag(&i, argc, argv, spec);
                } break;
                case ARGS_literal: {
                    LITERAL_TYPE *p =
                        (LITERAL_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_literal(&i, argc, argv, spec);
                } break;
                case ARGS_string: {
                    STRING_TYPE *p =
                        (STRING_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_string(&i, argc, argv, spec);
                } break;
                case ARGS_uint: {
                    UINT_TYPE *p =
                        (UINT_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_uint(&i, argc, argv, spec);
                } break;
                case ARGS_int: {
                    INT_TYPE *p =
                        (INT_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_int(&i, argc, argv, spec);
                } break;
                case ARGS_float: {
                    FLOAT_TYPE *p =
                        (FLOAT_TYPE *) (((char *)ret) + offset);
                    *p = ARGS_parse_float(&i, argc, argv, spec);
                } break;
                default:
                    assert( 0 && "unreachable" );
                    break;
            }
        } else if ( opt_index == spec.options_len ) {
            ARGS_implicit_help(spec, argv[0]);
            is_valid = 0;
            break;
        } else {
            if ( !(spec.flags & ARGS_SPECFLAGS_no_implicit_help) ) {
                ARGS_implicit_help(spec, argv[0]);
                is_valid = 0;
                break;
            } else {
                ARGS_log(spec,
                        "Warning: ignoring argument %d: '%s'\n",
                        i, argv[i]);
            }
        }
    }
    FLAG_TYPE *ARGS_valid_p =
        (FLAG_TYPE *) (((char *)ret) + valid_offset);
    *ARGS_valid_p = is_valid;
    return ret;
}

void ARGS_struct_from_spec(const ARGS_ArgSpec spec) {
    printf("typedef struct {\n");
    for ( size_t i = 0; i < spec.options_len; i++ ) {
        const ARGS_ArgOption option = spec.options[i];
        printf("    %s ", ARGS_typename(option.type));
        for ( const char *cp = option.big; *cp; cp++ ) {
            printf("%c", ( *cp == '-' ) ? '_' : *cp);
        }
        printf(";\n");
    }
    printf("    %s ARGS_valid;\n", ARGS_typename(ARGS_flag));
    printf("} Options;\n");
}
