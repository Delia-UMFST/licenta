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
#include <map>
#include <string>

int plugin_is_GPL_compatible;

namespace {

const pass_data io_pass_data = {
    GIMPLE_PASS,
    "check_io_alternation",
    OPTGROUP_NONE,
    TV_NONE,
    PROP_gimple_any,
    0,
    0,
    0,
    0,
};

enum IOState {
    IO_UNKNOWN = 0,
    IO_INPUT = 1,
    IO_OUTPUT = 2
};

tree canonicalize_stream(tree arg) {
    while (arg && TREE_CODE(arg) == NOP_EXPR)
        arg = TREE_OPERAND(arg, 0);
    if (arg && (TREE_CODE(arg) == VAR_DECL || TREE_CODE(arg) == PARM_DECL))
        return arg;
    return arg;
}

class check_io_alternation_pass : public gimple_opt_pass {
    std::map<tree, IOState> stream_states;

public:
    check_io_alternation_pass(gcc::context *ctx)
        : gimple_opt_pass(io_pass_data, ctx) {}

    bool gate(function *) override {
        return true;
    }

    unsigned int execute(function *fun) override {
        stream_states.clear();

        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                if (!is_gimple_call(stmt))
                    continue;

                tree fndecl = gimple_call_fndecl(stmt);
                const char *fn_name = fndecl ? IDENTIFIER_POINTER(DECL_NAME(fndecl)) : "<unknown>";

                unsigned nargs = gimple_call_num_args(stmt);
                if (nargs == 0)
                    continue;

                tree file_arg = nullptr;
                if ((strcmp(fn_name, "__builtin_fwrite") == 0 || strcmp(fn_name, "__builtin_fread") == 0) && nargs >= 4) {
                    file_arg = gimple_call_arg(stmt, 3);
                } else {
                    file_arg = gimple_call_arg(stmt, 0);
                }

                if (!file_arg)
                    continue;

                tree canon_stream = canonicalize_stream(file_arg);

                if (strcmp(fn_name, "fflush") == 0 || strcmp(fn_name, "fseek") == 0 ||
                    strcmp(fn_name, "fsetpos") == 0 || strcmp(fn_name, "rewind") == 0) {
                    if (stream_states.count(canon_stream)) {
                        stream_states.erase(canon_stream);
                    }
                    continue;
                }

                IOState new_state = IO_UNKNOWN;
                if (strcmp(fn_name, "fprintf") == 0 || strcmp(fn_name, "fputs") == 0 ||
                    strcmp(fn_name, "fputc") == 0 || strcmp(fn_name, "__builtin_fwrite") == 0) {
                    new_state = IO_OUTPUT;
                } else if (strcmp(fn_name, "fscanf") == 0 || strcmp(fn_name, "fgets") == 0 ||
                           strcmp(fn_name, "fgetc") == 0 || strcmp(fn_name, "getc") == 0 ||
                           strcmp(fn_name, "getc_unlocked") == 0 || strcmp(fn_name, "__builtin_fread") == 0) {
                    new_state = IO_INPUT;
                }

                if (new_state == IO_UNKNOWN)
                    continue;

                IOState last_state = IO_UNKNOWN;
                if (stream_states.count(canon_stream))
                    last_state = stream_states[canon_stream];

                if (last_state != IO_UNKNOWN && last_state != new_state) {
                    if (last_state == IO_INPUT && new_state == IO_OUTPUT) {
                        warning_at(gimple_location(stmt), 0,
                                   "Plugin warning: stream used for output after input without intervening flush or repositioning");
                    } else if (last_state == IO_OUTPUT && new_state == IO_INPUT) {
                        warning_at(gimple_location(stmt), 0,
                                   "Plugin warning: stream used for input after output without intervening flush or repositioning");
                    }
                }

                stream_states[canon_stream] = new_state;
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
    pass_info.pass = new check_io_alternation_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP,
                      nullptr, &pass_info);

    return 0;
}
