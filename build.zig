const std = @import("std");

fn guardAppend(out: *std.ArrayList(u8), gpa: std.mem.Allocator, dst_base: []const u8) !void {
    for (dst_base) |c| {
        try out.append(gpa, switch (c) {
            'a'...'z' => c - 'a' + 'A',
            '.', '-' => '_',
            else => c,
        });
    }
}

fn escapeAppend(out: *std.ArrayList(u8), gpa: std.mem.Allocator, line: []const u8) !void {
    try out.appendSlice(gpa, " \"");
    for (line) |c| {
        switch (c) {
            '\\' => try out.appendSlice(gpa, "\\\\"),
            '"' => try out.appendSlice(gpa, "\\\""),
            else => try out.append(gpa, c),
        }
    }
    try out.appendSlice(gpa, "\\n\"");
}

fn pk2h(b: *std.Build, io: std.Io, src: []const u8, dst: []const u8, prefix: []const u8) !void {
    const gpa = b.allocator;
    const code = try std.Io.Dir.cwd().readFileAlloc(io, src, gpa, .limited(16 * 1024 * 1024));
    defer gpa.free(code);

    const dst_base = std.Io.Dir.path.basename(dst);

    var out: std.ArrayList(u8) = .empty;
    defer out.deinit(gpa);

    try out.appendSlice(gpa, "#ifndef ");
    try guardAppend(&out, gpa, dst_base);
    try out.appendSlice(gpa, "\n#define ");
    try guardAppend(&out, gpa, dst_base);
    try out.appendSlice(gpa, "\n\n#define ");
    try out.appendSlice(gpa, prefix);
    try out.appendSlice(gpa, "_CODE \\\n");

    const body = if (std.mem.endsWith(u8, code, "\n")) code[0 .. code.len - 1] else code;

    var lines = std.mem.splitScalar(u8, body, '\n');
    var first = true;
    while (lines.next()) |line| {
        if (first) {
            first = false;
        } else {
            try out.appendSlice(gpa, " \\\n");
        }
        try escapeAppend(&out, gpa, line);
    }

    try out.appendSlice(gpa, "\n\n#endif // ");
    try guardAppend(&out, gpa, dst_base);
    try out.appendSlice(gpa, "\n");

    const old = std.Io.Dir.cwd().readFileAlloc(io, dst, gpa, .limited(16 * 1024 * 1024)) catch null;
    if (old) |o| {
        defer gpa.free(o);
        if (std.mem.eql(u8, o, out.items)) return;
    }

    try std.Io.Dir.cwd().writeFile(io, .{
        .sub_path = dst,
        .data = out.items,
    });
}

pub fn build(b: *std.Build) void {
    var threaded: std.Io.Threaded = .init(b.allocator, .{});
    defer threaded.deinit();
    const io = threaded.io();

    pk2h(b, io, "repl.pk", "repl.h", "REPL") catch |err| {
        std.debug.print("pk2h: repl.pk -> repl.h failed: {t}\n", .{err});
        std.process.exit(1);
    };
    pk2h(b, io, "src/lisp/std.pk", "src/pocket_std.h", "PK_STD") catch |err| {
        std.debug.print("pk2h: src/lisp/std.pk -> src/pocket_std.h failed: {t}\n", .{err});
        std.process.exit(1);
    };

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

    b.installArtifact(exe);

    const run_cmd = b.step("run", "Run the repl");
    const run = b.addRunArtifact(exe);
    if (b.args) |args| {
        run.addArgs(args);
    }
    run_cmd.dependOn(&run.step);
}
