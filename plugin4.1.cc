#include "gcc-plugin.h"
#include "plugin-version.h"
#include "system.h"
#include "context.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-pass.h"
#include "function.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-ssa.h"

int plugin_is_GPL_compatible;

namespace {

const pass_data pointer_arithmetic_pass_data = {
    GIMPLE_PASS,
    "warn_pointer_arithmetic",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0
};

tree track_origin(tree ptr) {
    int depth = 0;
    while (ptr && depth++ < 100) {
        if (TREE_CODE(ptr) == SSA_NAME) {
            gimple* def_stmt = SSA_NAME_DEF_STMT(ptr);
            if (!def_stmt || !is_gimple_assign(def_stmt)) break;
            ptr = gimple_assign_rhs1(def_stmt);
        }
        else if (TREE_CODE(ptr) == ADDR_EXPR) {
            ptr = TREE_OPERAND(ptr, 0);
        }
        else if (TREE_CODE(ptr) == VAR_DECL) {
            if (DECL_INITIAL(ptr)) {
                ptr = DECL_INITIAL(ptr);
                continue;
            }
            break;
        }
        else if (TREE_CODE(ptr) == ARRAY_REF) {
            ptr = TREE_OPERAND(ptr, 0);
        }
		else if (TREE_CODE(ptr) == MEM_REF || TREE_CODE(ptr) == BIT_FIELD_REF) {
			ptr = TREE_OPERAND(ptr, 0); // Might come from a pointer
		}
        else {
            break;
        }
    }
    return ptr;
}

bool is_array_origin(tree ptr) {
    tree origin = track_origin(ptr);
    return origin && TREE_CODE(origin) == VAR_DECL && 
           TREE_CODE(TREE_TYPE(origin)) == ARRAY_TYPE;
}

class pointer_arithmetic_pass : public gimple_opt_pass {
public:
    pointer_arithmetic_pass(gcc::context *ctx)
        : gimple_opt_pass(pointer_arithmetic_pass_data, ctx) {}

    bool gate(function *) override { return true; }

    unsigned int execute(function *fun) override {
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                if (is_gimple_assign(stmt) && gimple_assign_rhs_code(stmt) == POINTER_PLUS_EXPR) {
                    tree ptr = gimple_assign_rhs1(stmt);
                    location_t loc = gimple_location(stmt);
                    if (!is_array_origin(ptr)) {
                        tree var = ptr;
						if (TREE_CODE(var) == SSA_NAME)
							var = SSA_NAME_VAR(var);
						warning_at(loc, 0, "Plugin warning: pointer arithmetic on non-array object '%qE' of type '%qT'", var, TREE_TYPE(var));

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
    pass_info.pass = new pointer_arithmetic_pass(g);
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      nullptr,
                      &pass_info);

    return 0;
}
