

set(CAPSTONE_BUILD_STATIC_MSVC_RUNTIME ON)
add_subdirectory(capstone)

set_target_properties(capstone PROPERTIES FOLDER "third_party/capstone")
set_target_properties(capstone_static PROPERTIES FOLDER "third_party/capstone")
set_target_properties(fuzz_disasm PROPERTIES FOLDER "third_party/capstone")