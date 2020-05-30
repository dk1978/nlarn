import os

Import('env')
pEnv = env.Clone()

if pEnv['TOOLCHAIN'] == 'win32':
    if GetOption('debug_build'):
        pEnv.Append(CPPDEFINES = 'NLARN_DEBUG')
        pEnv.Append(CFLAGS = '/sdl /EHsc /Od /MDd /Z7 /FS /Fdbuild\\')
        pEnv.Append(LINKFLAGS = '/debug')
    if GetOption('release_build'):
        pEnv.Append(CFLAGS = '/sdl /EHsc /MD /O2')
    pEnv.Append(CPPDEFINES = ['H5_BUILT_AS_DYNAMIC_LIB'])
elif pEnv['TOOLCHAIN'] == 'mingw':
    if GetOption('debug_build'):
        pEnv.Append(CPPDEFINES = 'NLARN_DEBUG')
        pEnv.Append(CFLAGS = '-Wall -Wextra -Werror -g3 -O0')
    if GetOption('coverage_build'):
        pEnv.Append(CPPDEFINES = 'NLARN_DEBUG')
        pEnv.Append(CFLAGS = '-Wall -Wextra -Werror -g3 -O0 --coverage')
        pEnv.Append(LINKFLAGS = '--coverage')
    if GetOption('release_build'):
        pEnv.Append(CFLAGS = '-Wall -Wextra -Werror -O2')
    pEnv.Append(CPPPATH = "/opt/mxe/usr/x86_64-w64-mingw32.shared/include")
else:
    if GetOption('debug_build'):
        pEnv.Append(CPPDEFINES = 'NLARN_DEBUG')
        pEnv.Append(CFLAGS = '-std=c99 -Wall -Wextra -Werror -g3 -O0')
    if GetOption('coverage_build'):
        pEnv.Append(CPPDEFINES = 'NLARN_DEBUG')
        pEnv.Append(CFLAGS = '-std=c99 -Wall -Wextra -Werror -g3 -O0 --coverage')
        pEnv.Append(LINKFLAGS = '--coverage')
    if GetOption('release_build'):
        pEnv.Append(CFLAGS = '-std=c99 -Wall -Wextra -Werror -O2')

pEnv.Append(CPPPATH = ['inc', 'inc/external'])
sources = Glob('src/*.c')
sources.extend(Glob('src/external/*.c'))

pEnv.Program('nlarn', sources)
