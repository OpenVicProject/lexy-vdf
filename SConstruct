#!/usr/bin/env python

import os
import platform
import sys

import SCons

BINDIR = "bin"

env = SConscript("scripts/SConstruct")

env.PrependENVPath("PATH", os.getenv("PATH"))

opts = env.SetupOptions()

opts.Add(BoolVariable(key="build_lvdf_library", help="Build the lexy vdf library.", default=env.get("build_lvdf_library", not env.is_standalone)))
opts.Add(BoolVariable("build_lvdf_headless", "Build the lexy vdf headless executable", env.is_standalone))

env.FinalizeOptions()

SConscript("deps/SCsub", "env")

env.lexy_vdf = {}

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
source_path = "src/lexy-vdf"
include_path = "include"
env.Append(CPPPATH=[[env.Dir(p) for p in [source_path, include_path]]])
sources = env.GlobRecursive("*.cpp", [source_path])
env.lexy_vdf_sources = sources

suffix = ".{}.{}".format(env["platform"], env["target"])
if env.dev_build:
    suffix += ".dev"
if env["precision"] == "double":
    suffix += ".double"
suffix += "." + env["arch"]

# Expose it when included from another project
env["suffix"] = suffix

library = None
env["OBJSUFFIX"] = suffix + env["OBJSUFFIX"]
library_name = "liblexy-vdf{}{}".format(suffix, env["LIBSUFFIX"])

if env["build_lvdf_library"]:
    library = env.StaticLibrary(target=os.path.join(BINDIR, library_name), source=sources)
    Default(library)

    env.Append(LIBPATH=[env.Dir(BINDIR)])
    env.Prepend(LIBS=[library_name])

    env.lexy_vdf["LIBPATH"] = env["LIBPATH"]
    env.lexy_vdf["LIBS"] = env["LIBS"]
    env.lexy_vdf["INCPATH"] = [env.Dir(include_path)]

headless_program = None
env["PROGSUFFIX"] = suffix + env["PROGSUFFIX"]

if env["build_lvdf_headless"]:
    headless_name = "lexy-vdf"
    headless_env = env.Clone()
    headless_path = ["src/headless"]
    headless_env.Append(CPPDEFINES=["LEXY_VDF_HEADLESS"])
    headless_env.Append(CPPPATH=[headless_env.Dir(headless_path)])
    headless_env.headless_sources = env.GlobRecursive("*.cpp", headless_path)
    if not env["build_lvdf_headless"]:
        headless_env.headless_sources += sources
    headless_program = headless_env.Program(
        target=os.path.join(BINDIR, headless_name),
        source=headless_env.headless_sources,
        PROGSUFFIX=".headless" + env["PROGSUFFIX"]
    )
    Default(headless_program)


if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    env.show_progress(env)

Return("env")