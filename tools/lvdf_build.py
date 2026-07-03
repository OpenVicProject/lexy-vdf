def exists(env):
    return True


def options(opts):
    from SCons.Variables import BoolVariable

    opts.Add(BoolVariable("build_lvdf_library", "Build the lexy vdf library.", False))
    opts.Add(BoolVariable("build_lvdf_headless", "Build the lexy vdf headless executable", True))


def generate(env):
    if not env.is_standalone:
        env["build_lvdf_library"] = True
        env["build_lvdf_headless"] = False
