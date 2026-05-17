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

inc = include_directories('include')
gperf_inc = include_directories('gperf')
incdirs = [inc, gperf_inc]

executable(
    'semitov-riscv-as',
    sources,
    include_directories: incdirs,
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

clangformat = find_program('clang-format', required: false)
if clangformat.found()
    run_target('format', command: ['clang-format', '-i'] + sources)
endif