const std = @import("std");

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

fn guardAppend(out: *std.ArrayList(u8), gpa: std.mem.Allocator, dst: []const u8) !void {
    for (dst) |c| {
        try out.append(gpa, switch (c) {
            'a'...'z' => c - 'a' + 'A',
            '.', '-' => '_',
            else => c,
        });
    }
}

pub fn main(init: std.process.Init) !void {
    const gpa = init.gpa;
    const io = init.io;
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    // pk2h <src> <dst> <prefix>
    if (args.len != 4) {
        std.debug.print("usage: pk2h <src> <dst> <prefix>\n", .{});
        return error.Usage;
    }
    const src = args[1];
    const dst = args[2];
    const dst_base = std.Io.Dir.path.basename(dst);
    const prefix = args[3];

    const code = try std.Io.Dir.cwd().readFileAlloc(io, src, init.arena.allocator(), .limited(16 * 1024 * 1024));

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

    try std.Io.Dir.cwd().writeFile(io, .{
        .sub_path = dst,
        .data = out.items,
    });
}
