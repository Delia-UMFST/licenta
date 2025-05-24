#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "tree-pass.h"
#include "function.h"
#include "context.h"
#include "basic-block.h"
#include "diagnostic.h"

int plugin_is_GPL_compatible;

namespace {

const pass_data my_pass_data = {
    GIMPLE_PASS,
    "warn_mixed_bitwise",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0,
};

//Resolve actual type by walking SSA chain
tree resolve_operand_type(tree op) {
    while (TREE_CODE(op) == SSA_NAME) {
        gimple *def = SSA_NAME_DEF_STMT(op);
        if (!def || !is_gimple_assign(def)) break;
        op = gimple_assign_rhs1(def);
    }
    return TREE_TYPE(op);
}

class warn_mixed_bitwise_pass : public gimple_opt_pass {
public:
    warn_mixed_bitwise_pass(gcc::context *ctx) : gimple_opt_pass(my_pass_data, ctx) {}

    bool gate(function *) override {
        return true;
    }

    unsigned int execute(function *fun) override {
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                if (!is_gimple_assign(stmt)) continue;

                enum tree_code code = gimple_assign_rhs_code(stmt);
                if (code == BIT_AND_EXPR || code == BIT_IOR_EXPR || code == BIT_XOR_EXPR ||
					code == LSHIFT_EXPR || code == RSHIFT_EXPR) {
                    tree op1 = gimple_assign_rhs1(stmt);
                    tree op2 = gimple_assign_rhs2(stmt);

                    tree t1 = resolve_operand_type(op1);
                    tree t2 = resolve_operand_type(op2);

                    if (t1 && t2 &&
                        TREE_CODE(t1) == INTEGER_TYPE &&
                        TREE_CODE(t2) == INTEGER_TYPE) {

                        bool t1_signed = TYPE_UNSIGNED(t1) == 0;
                        bool t2_signed = TYPE_UNSIGNED(t2) == 0;

                        if (t1_signed != t2_signed) {
                            location_t loc = gimple_location(stmt);
                            warning_at(gimple_location(stmt), 0,
                                       "Plugin warning: mixed signedness in bitwise op (op1: %s, op2: %s)",
                                       t1_signed ? "signed" : "unsigned",
                                       t2_signed ? "signed" : "unsigned");
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
    pass_info.pass = new warn_mixed_bitwise_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, nullptr, &pass_info);

    return 0;
}

