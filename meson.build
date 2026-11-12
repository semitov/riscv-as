project(
    'semitov-riscv-as',
    'c',
    version: '0.2',
    default_options: ['warning_level=3', 'c_std=c11'],
)

sources = []
headers = []
subdir('include')
subdir('src')
subdir('gperf')


add_global_arguments('-DEVELOPER', language: 'c')
add_global_arguments('-DINFO', language: 'c')


unity = subproject('unity')
unity_dependency = unity.get_variable('unity_dep')
unity_gen_runner = unity.get_variable('gen_test_runner')

inc = include_directories('include')
gperf_inc = include_directories('gperf')
incdirs = [inc, gperf_inc]

executable(
    'semitov-riscv-as',
    sources,
    include_directories: incdirs,
)

test1_src = files([
  'src' / 'instruction.c',
  'src' / 'utils.c',
  'src' / 'writer.c',
  'src' / 'symtable.c',
  'src' / 'debug.c',
  'gperf' / 'hash.c',
  'test' / 'src' / 'handle' / 'r_type' / 'test_handle_add.c'
])

test1 = executable('test1',
  sources: [
    test1_src,
    unity_gen_runner.process('test' / 'src' / 'handle' / 'r_type' / 'test_handle_add.c')
  ],
  include_directories: [ incdirs ],
  dependencies: [ unity_dependency ],
)

test('test1', test1, should_fail: false)

gperf = find_program('gperf', required: true)
if gperf.found()
    run_command(
        [
            gperf,
            '-o',
            '-t',
            '-I',
            '-C',
            '--output-file='
            + join_paths(meson.current_source_dir(), 'gperf/hash.c'),
            join_paths(meson.current_source_dir(), 'gperf/instructions.gperf'),
        ],
        check: true,
    )
endif

clangformat = find_program('clang-format', required: false)
if clangformat.found()
    run_target('format', command: ['clang-format', '-i'] + sources)
endif
