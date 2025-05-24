#include "gcc-plugin.h"
#include "plugin-version.h"
#include "context.h"
#include "tree.h"
#include "tree-core.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-pass.h"
#include "function.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-ssa.h"
#include "tree-pretty-print.h"
#include "cpplib.h"
#include <vec.h>
#include <cstring>

int plugin_is_GPL_compatible;

namespace {

const pass_data string_terminator_pass_data = {
    GIMPLE_PASS,
    "check_string_terminator",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0
};

const char* string_funcs[] = {
    "puts",
    "printf",
    "strcpy",
    "strncpy",
    "strlen",
    "strcmp",
    "strcat",
    "fputs",
    nullptr
};

bool is_string_function(const char* name) {
    for (int i = 0; string_funcs[i]; ++i) {
        if (strcmp(name, string_funcs[i]) == 0)
            return true;
    }
    return false;
}

static tree find_local_var_initializer(function *fun, tree var) {
    basic_block bb;
    FOR_EACH_BB_FN(bb, fun) {
        for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
            gimple *stmt = gsi_stmt(gsi);
            if (is_gimple_assign(stmt)) {
                tree lhs = gimple_assign_lhs(stmt);
                if (lhs == var) {
                    return gimple_assign_rhs1(stmt);
                }
            }
        }
    }
    return nullptr;
}

static bool is_potentially_unsafe_array(function *fun, tree arg) {
    if (TREE_CODE(arg) == ADDR_EXPR) {
        arg = TREE_OPERAND(arg, 0);
    }

    if (TREE_CODE(arg) != VAR_DECL) {
        return true;
    }

    tree type = TREE_TYPE(arg);
    if (TREE_CODE(type) != ARRAY_TYPE) {
        return true;
    }

    tree elem_type = TREE_TYPE(type);
    if (!(elem_type == char_type_node || elem_type == signed_char_type_node || elem_type == unsigned_char_type_node)) {
        return false;
    }

    tree domain = TYPE_DOMAIN(type);
    if (!domain || TREE_CODE(domain) != INTEGER_TYPE) {
        return true;
    }

    tree max = TYPE_MAX_VALUE(domain);
    tree min = TYPE_MIN_VALUE(domain);
    if (!max || !min || TREE_CODE(max) != INTEGER_CST || TREE_CODE(min) != INTEGER_CST) {
        return true;
    }

    unsigned HOST_WIDE_INT array_size = TREE_INT_CST_LOW(max) - TREE_INT_CST_LOW(min) + 1;

    tree init = DECL_INITIAL(arg);

    if (!init) {
        init = find_local_var_initializer(fun, arg);
        if (!init) {
            return true;
        }
    }

    if (TREE_CODE(init) == STRING_CST) {
        const char* str = TREE_STRING_POINTER(init);
        unsigned len = TREE_STRING_LENGTH(init);

        if (array_size == 0) {
            return true;
        }
        if (str[array_size - 1] != '\0') {
            return true;
        }

        return false;
    }

    if (TREE_CODE(init) == CONSTRUCTOR) {
        unsigned count = 0;
        bool has_null = false;
        tree idx, val;
        unsigned ix;

        FOR_EACH_CONSTRUCTOR_ELT(CONSTRUCTOR_ELTS(init), ix, idx, val) {
            ++count;
            if (TREE_CODE(val) == INTEGER_CST && TREE_INT_CST_LOW(val) == 0)
                has_null = true;
        }

        return !(has_null && array_size >= count);
    }

    return true;
}

class check_string_terminator_pass : public gimple_opt_pass {
public:
    check_string_terminator_pass(gcc::context *ctx)
        : gimple_opt_pass(string_terminator_pass_data, ctx) {}

    bool gate(function *) override { return true; }

    unsigned int execute(function *fun) override {
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);

                if (is_gimple_call(stmt)) {
                    tree callee = gimple_call_fndecl(stmt);
                    if (!callee || !DECL_NAME(callee)) continue;
                    const char* func_name = IDENTIFIER_POINTER(DECL_NAME(callee));
                    if (!is_string_function(func_name)) continue;

                    if (gimple_call_num_args(stmt) > 0) {
                        tree arg = gimple_call_arg(stmt, 0);

                        if (is_potentially_unsafe_array(fun, arg)) {
                            location_t loc = gimple_location(stmt);
                            warning_at(loc, 0,
                                "Plugin warning: passing potentially non-null-terminated array to '%s'", func_name);
                        }
                    }
                }
            }
        }

        return 0;
    }
};

}

extern "C" int plugin_init(struct plugin_name_args *plugin_info,
                           struct plugin_gcc_version *version) {
    if (!plugin_default_version_check(version, &gcc_version)) {
        error("Incompatible GCC version");
        return 1;
    }

    struct register_pass_info pass_info;
    pass_info.pass = new check_string_terminator_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      nullptr,
                      &pass_info);

    return 0;
}
