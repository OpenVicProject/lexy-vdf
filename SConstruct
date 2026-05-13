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

suffix = ".{}.{}".format(env["platform"], env["target"])
if env.dev_build:
    suffix += ".dev"
if env["precision"] == "double":
    suffix += ".double"
suffix += "." + env["arch"]
if env["platform"] == "windows":
    if env.get("debug_crt", False):
        suffix += ".mdd"
    elif env.get("use_static_cpp", False):
        suffix += ".mt"
    else:
        suffix += ".md"
if env.get("use_asan", False):
    suffix += ".san"
env["suffix"] = suffix

build_dir = env.Dir("build/" + suffix.lstrip(".")).abspath.replace("\\", "/")
env["build_dir"] = build_dir

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
# Out-of-source build: variant tree holds object files only, not copies of the
# source. Compile diagnostics therefore reference original source paths.
lexyvdf_variant = build_dir + "/" + source_path
env.VariantDir(lexyvdf_variant, source_path, duplicate=False)
env.Append(CPPPATH=[[env.Dir(p) for p in [include_path, lexyvdf_variant, source_path]]])
sources = env.GlobRecursiveVariant("*.cpp", source_path, lexyvdf_variant)
env.lexy_vdf_sources = sources

library = None
env["OBJSUFFIX"] = suffix + env["OBJSUFFIX"]
library_name = "liblexy-vdf{}{}".format(suffix, env["LIBSUFFIX"])

default_args = []

if env["build_lvdf_library"]:
    library = env.StaticLibrary(target=env.File(os.path.join(BINDIR, library_name)), source=sources)
    default_args += [library]

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
    headless_src = "src/headless"
    headless_variant = build_dir + "/" + headless_src
    headless_env.VariantDir(headless_variant, headless_src, duplicate=False)
    headless_env.Append(CPPDEFINES=["LEXY_VDF_HEADLESS"])
    headless_env.Append(CPPPATH=[headless_env.Dir(headless_variant), headless_env.Dir(headless_src)])
    headless_env.headless_sources = env.GlobRecursiveVariant("*.cpp", headless_src, headless_variant)
    if not env["build_lvdf_library"]:
        headless_env.headless_sources += sources
    headless_program = headless_env.Program(
        target=os.path.join(BINDIR, headless_name),
        source=headless_env.headless_sources,
        PROGSUFFIX=".headless" + env["PROGSUFFIX"]
    )
    default_args += [headless_program]

# Add compiledb if the option is set
if env.get("compiledb", False):
    default_args += ["compiledb"]

Default(*default_args)

if "env" in locals():
    # FIXME: This method mixes both cosmetic progress stuff and cache handling...
    env.show_progress(env)

Return("env")