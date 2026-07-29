cmake_minimum_required(VERSION 3.28)
project(lexy-vdf LANGUAGES CXX)

if(NOT COMMAND openvic_setup_base_flags)
    # scripts is the shared build-tooling repo, fetched once as a pinned
    # tarball (like the other third-party deps). In a composed build the
    # outermost repo fetches + loads it first and this guard skips for the
    # nested repos; a standalone repo fetches it itself. FetchContent also
    # dedups by name, so either path yields exactly one fetch.
    #
    # Raw FetchContent (not openvic_declare_dep) on purpose: openvic_declare_dep
    # lives INSIDE scripts (OpenVicDeps.cmake), so it isn't defined yet here.
    # Local-dev escape hatch: -DFETCHCONTENT_SOURCE_DIR_OPENVIC_SCRIPTS=<path>
    # points this at a working-tree scripts checkout.
    include(FetchContent)
    FetchContent_Declare(
        openvic_scripts
        URL "https://github.com/OpenVicProject/scripts/archive/8f83cabf147de7d8a511b4aaefd137777c4eb9c8.tar.gz"
        URL_HASH SHA256=5f9b1ff9e985e63be1b11fe8ba573023afb26c8e2811961f88c5ad2b8ac283da
    )
    FetchContent_MakeAvailable(openvic_scripts)
    include("${openvic_scripts_SOURCE_DIR}/cmake/OpenVicScripts.cmake")
endif()
openvic_setup_base_flags()
openvic_disable_rtti()

# A parent build (openvic-dataloader) declares lexy under the same FetchContent
# name; the first declaration wins globally, so in the composed build this pin
# is ignored and dataloader's copy is used. This pin only takes effect in a
# standalone lexy-vdf build.
set(LEXY_ENABLE_INSTALL OFF)
openvic_declare_dep(lexy
    REPO foonathan/lexy
    SHA 231bbb93faa07d635d7bb3742fb5365bb1800de3
    SHA256 2556ba90a5b384653a2cd56ce298e13c1c0f9a0ed1baee143289289a8cbc650d
)
FetchContent_MakeAvailable(lexy)
# Build lexy's file.cpp with the unicode database enabled; upstream lexy only
# puts the define on the lexy_unicode INTERFACE target.
target_compile_definitions(lexy_file PRIVATE LEXY_HAS_UNICODE_DATABASE=1)

file(GLOB_RECURSE lexy_vdf_sources CONFIGURE_DEPENDS src/lexy-vdf/*.cpp)

add_library(lexy-vdf STATIC ${lexy_vdf_sources})
add_library(openvic::lexy-vdf ALIAS lexy-vdf)

set(lvdf_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/gen-include)
target_include_directories(
    lexy-vdf
    PUBLIC include $<BUILD_INTERFACE:${lvdf_gen_dir}>
    PRIVATE src/lexy-vdf
)
target_link_libraries(lexy-vdf PRIVATE foonathan::lexy)

# Compile-time project info headers, included as <lexy-vdf/gen/*.gen.hpp>.
openvic_generate_commit_info(
    TARGET lexy-vdf
    PREFIX lvdf
    REPO_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT ${lvdf_gen_dir}/lexy-vdf/gen/commit_info.gen.hpp
)
openvic_generate_license_info(
    TARGET lexy-vdf
    PREFIX lvdf
    COPYRIGHT ${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
    LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    OUTPUT ${lvdf_gen_dir}/lexy-vdf/gen/license_info.gen.hpp
)
openvic_generate_author_info(
    TARGET lexy-vdf
    PREFIX lvdf
    AUTHORS ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS.md
    OUTPUT ${lvdf_gen_dir}/lexy-vdf/gen/author_info.gen.hpp
    SECTIONS
        "Senior Developers=AUTHORS_SENIOR_DEVELOPERS"
        "Developers=AUTHORS_DEVELOPERS"
        "Contributors=AUTHORS_CONTRIBUTORS"
        "Consultants=AUTHORS_CONSULTANTS"
)
set_target_properties(lexy-vdf PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# Headless executable: built by default only when lexy-vdf is the top-level
# project (composed builds just want the library).
option(LEXY_VDF_BUILD_HEADLESS "Build the lexy-vdf headless executable" ${PROJECT_IS_TOP_LEVEL})
if(LEXY_VDF_BUILD_HEADLESS)
    file(GLOB_RECURSE lexy_vdf_headless_sources CONFIGURE_DEPENDS src/headless/*.cpp)
    add_executable(lexy-vdf-headless ${lexy_vdf_headless_sources})
    target_compile_definitions(lexy-vdf-headless PRIVATE LEXY_VDF_HEADLESS)
    target_include_directories(lexy-vdf-headless PRIVATE src/headless)
    target_link_libraries(lexy-vdf-headless PRIVATE openvic::lexy-vdf)
    set_target_properties(
        lexy-vdf-headless
        PROPERTIES
            OUTPUT_NAME "lexy-vdf.headless"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    )
endif()
