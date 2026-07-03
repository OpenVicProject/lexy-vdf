#!/usr/bin/env python

import os

BINDIR = "bin"

env = SConscript("scripts/SConstruct", exports={"gen_dir": "include/lexy-vdf/gen"})

env.PrependENVPath("PATH", os.getenv("PATH"))

if env.is_standalone:
    env.VariantDir(env["build_dir"], env.Dir("."), duplicate=False)

SConscript("deps/SCsub", "env", variant_dir=env["build_dir"].Dir("deps"), duplicate=False)

env["name_prefix"] = "lvdf"
gen_commit_info = env.Git(
    "commit_info.gen.hpp",
    env.Value(env.GetGitInfo()),
)
Default(gen_commit_info)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

env.AddLibraryIncludes("include", add_variant_dir=True)
env.AddLibraryIncludes("src", False)
env.AddLibraryIncludes("src/lexy-vdf", False)
env.AddLibrarySources("src/lexy-vdf")

if env["build_lvdf_library"]:
    env.BuildBaseLibrary(os.path.join(BINDIR, "liblexy-vdf"))

if env["build_lvdf_headless"]:
    env.BuildHeadlessProgram(
        target=os.path.join(BINDIR, "lexy-vdf"),
        src_dir="src/headless",
        defines_prefix="lexy_vdf",
        include_lib_src=not env["build_lvdf_library"],
    )

Return("env")
