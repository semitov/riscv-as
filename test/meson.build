unity = subproject('unity')
unity_dependency = unity.get_variable('unity_dep')
unity_gen_runner = unity.get_variable('gen_test_runner')

# Map of tests names -> source path is relative to test/
unit_tests = {
    'handle_add':  'src' / 'handle' / 'r_type' / 'test_handle_add.c',
    'handle_addi': 'src' / 'handle' / 'i_type' / 'test_handle_addi.c',
}

foreach name, test_file : unit_tests
    test_exe = executable(
        'test_' + name,
        sources: [
            test_file,
            unity_gen_runner.process(test_file),
        ],
        include_directories: incdirs,
        link_with: assembler_core_lib,
        dependencies: [unity_dependency],
    )

    test(name, test_exe)
endforeach