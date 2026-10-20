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

fn generatePocketLispSrcHeader_(b: *std.Build, io: std.Io, src: []const u8, dst: []const u8, prefix: []const u8) !void {
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

fn generatePocketLispSrcHeader(b: *std.Build, io: std.Io, src: []const u8, dst: []const u8, prefix: []const u8) void {
    generatePocketLispSrcHeader_(b, io, src, dst, prefix) catch |err| {
        std.debug.panic("generatePocketLispSrcHeader(): {s} => {s}, failed: {}", .{ src, dst, err });
    };
}

pub fn build(b: *std.Build) void {
    {
        var threaded: std.Io.Threaded = .init(b.allocator, .{});
        defer threaded.deinit();

        const io = threaded.io();

        generatePocketLispSrcHeader(b, io, "repl/repl.pk", "repl/repl.h", "REPL");
        generatePocketLispSrcHeader(b, io, "src/lisp/std.pk", "src/pocket_std.h", "PK_STD");
    }
    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});

    const root = b.createModule(.{
        .target = target,
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

    const srcFiles = [_][]const u8{
        "src/pocket_array.c",
        "src/pocket_atom.c",
        "src/pocket_blob.c",
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
    };

    const flags = [_][]const u8{
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-Werror",
        optimize_flag,
    };
    const exeFiles = [_][]const u8{"repl/repl.c"} ++ srcFiles;
    root.addCSourceFiles(.{
        .files = &exeFiles,
        .flags = &flags,
    });

    const static_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    static_mod.addCSourceFiles(.{ .files = &srcFiles, .flags = &flags });
    const static_lib = b.addLibrary(.{
        .name = "pocket",
        .linkage = .static,
        .root_module = static_mod,
    });

    const shared_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    
    const shared_flags = [_][]const u8{
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-Werror",
        optimize_flag,
        "-fvisibility=hidden"
    };
    
    shared_mod.addCSourceFiles(.{ .files = &srcFiles, .flags = &shared_flags });
    shared_mod.addCMacro("PK_CFLAG_SHARED", "1");
    
    const shared_lib = b.addLibrary(.{
        .name = "pocket",
        .linkage = .dynamic,
        .root_module = shared_mod,
    });

    if (optimize == .Debug) {
        root.addCMacro("PK_CFLAG_DEBUG", "1");
        static_mod.addCMacro("PK_CFLAG_DEBUG", "1");
        shared_mod.addCMacro("PK_CFLAG_DEBUG", "1");
    }

    b.installArtifact(exe);
    b.installArtifact(static_lib);
    b.installArtifact(shared_lib);

    const run_cmd = b.step("run", "Run the repl");
    const run = b.addRunArtifact(exe);
    if (b.args) |args| {
        run.addArgs(args);
    }
    run_cmd.dependOn(&run.step);
}
