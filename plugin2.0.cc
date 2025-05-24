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
#include "tree-cfg.h"
#include "cgraph.h"
#include <set>

int plugin_is_GPL_compatible;

namespace {

const pass_data uaf_pass_data = {
    GIMPLE_PASS,
    "detect_uaf",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0,
};

class detect_uaf_pass : public gimple_opt_pass {
public:
    detect_uaf_pass(gcc::context *ctx)
        : gimple_opt_pass(uaf_pass_data, ctx) {}

    bool gate(function *) override {
        return true;
    }

    unsigned int execute(function *fun) override {
        std::set<tree> freed_pointers;

        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb);
                 !gsi_end_p(gsi); gsi_next(&gsi)) {

                gimple *stmt = gsi_stmt(gsi);

                if (is_gimple_call(stmt)) {
                    tree fndecl = gimple_call_fndecl(stmt);
                    if (fndecl && strcmp(IDENTIFIER_POINTER(DECL_NAME(fndecl)), "free") == 0) {
                        if (gimple_call_num_args(stmt) == 1) {
                            tree arg = gimple_call_arg(stmt, 0);
                            freed_pointers.insert(arg);
                        }
                    }
                }

                if (is_gimple_assign(stmt)) {
                    tree rhs1 = gimple_assign_rhs1(stmt);
                    tree lhs = gimple_assign_lhs(stmt);

                    if (TREE_CODE(lhs) == INDIRECT_REF || TREE_CODE(lhs) == MEM_REF) {
                        tree ptr = TREE_OPERAND(lhs, 0);
                        if (freed_pointers.count(ptr)) {
                            warning_at(gimple_location(stmt), 0,
                                       "Plugin warning: writing to freed pointer");
                        }
                    }

                    if (TREE_CODE(rhs1) == INDIRECT_REF || TREE_CODE(rhs1) == MEM_REF) {
                        tree ptr = TREE_OPERAND(rhs1, 0);
                        if (freed_pointers.count(ptr)) {
                            warning_at(gimple_location(stmt), 0,
                                       "Plugin warning: reading from freed pointer");
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
    pass_info.pass = new detect_uaf_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP,
                      nullptr, &pass_info);

    return 0;
}

