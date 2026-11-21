project(
    'semitov-riscv-as',
    'c',
    version: '0.2',
    default_options: ['warning_level=3', 'c_std=c11'],
)

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

sources = []
headers = []
subdir('include')
subdir('src')
subdir('gperf')


add_global_arguments('-DEVELOPER', language: 'c')
add_global_arguments('-DINFO', language: 'c')

inc = include_directories('include')
gperf_inc = include_directories('gperf')
incdirs = [inc, gperf_inc]

executable(
    'semitov-riscv-as',
    sources,
    include_directories: incdirs,
)

test_core_src = files([
  'src' / 'instruction.c',
  'src' / 'utils.c',
  'src' / 'writer.c',
  'src' / 'symtable.c',
  'src' / 'debug.c',
  'gperf' / 'hash.c'
])

assembler_core_lib = static_library(
    'semitov_core',
    test_core_src,
    include_directories: incdirs,
)

subdir('test')

clangformat = find_program('clang-format', required: false)
if clangformat.found()
    run_target('format', command: ['clang-format', '-i'] + sources)
endif
