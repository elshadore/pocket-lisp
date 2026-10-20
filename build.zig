const std = @import("std");

pub fn build(b: *std.Build) void {
    const exe = b.addExecutable(.{
        .name = "pocket",
    });

    b.installArtifact(exe);
}
