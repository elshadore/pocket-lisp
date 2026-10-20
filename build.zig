const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .Debug });
    const debug = b.option(bool, "debug", "Enable PK_CFLAG_DEBUG") orelse true;

    const root = b.createModule(.{
        .target = b.resolveTargetQuery(.{}),
        .optimize = optimize,
        .link_libc = true,
    });

    const exe = b.addExecutable(.{
        .name = "pocket",
        .root_module = root,
    });

    const optimize_flag: []const u8 = switch (optimize) {
        .Debug => "-O0",
        .ReleaseFast => "-O3",
        .ReleaseSmall => "-Os",
        else => "-O2",
    };

    const flags = [_][]const u8{
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-Werror",
        optimize_flag,
    };
    root.addCSourceFiles(.{
        .files = &.{
            "repl.c",
            "src/pocket_atom.c",
            "src/pocket_call.c",
            "src/pocket_compiler.c",
            "src/pocket_core.c",
            "src/pocket_debug.c",
            "src/pocket_env.c",
            "src/pocket_eval.c",
            "src/pocket_frame.c",
            "src/pocket_fs.c",
            "src/pocket_function.c",
            "src/pocket_gc.c",
            "src/pocket_hashmap.c",
            "src/pocket_ident.c",
            "src/pocket_list.c",
            "src/pocket_macro.c",
            "src/pocket_math.c",
            "src/pocket_mem.c",
            "src/pocket_method.c",
            "src/pocket_object.c",
            "src/pocket_print.c",
            "src/pocket_read.c",
            "src/pocket_slotmap.c",
            "src/pocket_stack.c",
            "src/pocket_std.c",
            "src/pocket_string.c",
            "src/pocket_strsub.c",
            "src/pocket_symbol.c",
            "src/pocket_text.c",
            "src/pocket_throw.c",
            "src/pocket_type.c",
            "src/pocket_user.c",
            "src/pocket_utils.c",
            "src/pocket_vm.c",
            "src/pocket_writer.c",
        },
        .flags = &flags,
    });

    if (debug) {
        root.addCMacro("PK_CFLAG_DEBUG", "1");
    }

    const pk2h = b.addExecutable(.{
        .name = "pk2h",
        .root_module = b.createModule(.{
            .target = b.resolveTargetQuery(.{}),
            .root_source_file = b.path("tools/pk2h.zig"),
        }),
    });

    const gen_repl = b.addRunArtifact(pk2h);
    gen_repl.addArgs(&.{ "repl.pk", "repl.h", "REPL" });
    gen_repl.addFileInput(b.path("repl.pk"));

    const gen_std = b.addRunArtifact(pk2h);
    gen_std.addArgs(&.{ "src/lisp/std.pk", "src/pocket_std.h", "PK_STD" });
    gen_std.addFileInput(b.path("src/lisp/std.pk"));

    exe.step.dependOn(&gen_repl.step);
    exe.step.dependOn(&gen_std.step);

    b.installArtifact(exe);

    const run_cmd = b.step("run", "Run the repl");
    const run = b.addRunArtifact(exe);
    if (b.args) |args| {
        run.addArgs(args);
    }
    run_cmd.dependOn(&run.step);
}
