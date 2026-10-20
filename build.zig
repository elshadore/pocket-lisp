const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .Debug });
    const debug = b.option(bool, "debug", "Enable POCKET_CFLAG_DEBUG") orelse true;

    const root = b.createModule(.{
        .target = b.resolveTargetQuery(.{}),
        .optimize = optimize,
        .link_libc = true,
    });

    const exe = b.addExecutable(.{
        .name = "pocket",
        .root_module = root,
    });

    const flags = [_][]const u8{
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-O0",
    };
    root.addCSourceFile(.{ .file = b.path("repl.c"), .flags = &flags });

    if (debug) {
        root.addCMacro("POCKET_CFLAG_DEBUG", "1");
    }

    const gen_repl = b.addSystemCommand(&.{ "./tools/pk2h.sh", "repl.pk", "repl.h", "REPL" });
    const gen_std = b.addSystemCommand(&.{ "./tools/pk2h.sh", "src/lisp/std.pk", "src/pocket_std.h", "PK_STD" });
    exe.step.dependOn(&gen_repl.step);
    exe.step.dependOn(&gen_std.step);
    b.installArtifact(exe);

    const run_cmd = b.step("run", "Run the repl");
    const run = b.addRunArtifact(exe);
    if (b.args) |args| {
        run.addArgs(args);
    }
    run.step.dependOn(b.getInstallStep());
    run_cmd.dependOn(&run.step);
}
