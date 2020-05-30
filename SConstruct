#! /bin/python
import os

AddOption('--toolchain',
          action='store',
          type='choice',
          choices=['mingw_mxe','clang','gcc'],
          help='select the toolchain to use for the build')

AddOption('--debug-build',
          action='store_true',
          help='build in debug mode')

AddOption('--release-build',
          action='store_true',
          help='build in release mode')

AddOption('--coverage-build',
          action='store_true',
          help='build for coverage analysis')

env = Environment()
env['TOOLCHAIN'] = GetOption('toolchain')

if env['TOOLCHAIN'] == 'mingw_mxe':
    env.Tool('mingw')
    env['CXX'] = '/opt/mxe/usr/bin/x86_64-w64-mingw32.shared-g++'
elif env['TOOLCHAIN'] == 'clang':
    env['CXX'] = 'clang++-6.0'
    env['CC'] = 'clang-6.0'
elif env['TOOLCHAIN'] == 'gcc':
    env['CXX'] = 'g++-7'
    env['CC'] = 'gcc-7'
else:
    env['TOOLCHAIN'] = 'unknown'

outputDir = 'build'
if GetOption('debug_build'):
    outputDir = outputDir + '/' + env['TOOLCHAIN'] + '-debug'
if GetOption('release_build'):
    outputDir = outputDir + '/' + env['TOOLCHAIN'] + '-release'
if GetOption('coverage_build'):
    outputDir = outputDir + '/' + env['TOOLCHAIN'] + '-coverage'
env['OUTPUT_DIR'] = outputDir

# Specific build settings such as paths to third party tools and libraries
SConscript("SConsettingsDefault", exports={'env': env})
SConscript("SConsettings", exports={'env': env})


# place the sconsign file in the outputDir to avoid polluting the source dir
env.SConsignFile(outputDir + '/scons-signatures')

# build code targets
VariantDir(outputDir, ".", duplicate=0)
SConscript(outputDir + "/SConscript", exports={'env': env})

# copy lib files to outputDir
libFiles = Glob('lib/*')
Install(outputDir + '/lib', libFiles)

# Fix up include path and config variable files for Qt Creator IDE
includeFile = open("nlarn.includes", "w")
configFile = open("nlarn.config", "w")
includeFile.write('inc\ninc/external\n')
for setting,value in env.Dictionary().items():
    if 'CPPPATH' in setting:
            if type(value) is str and len(value) > 0:
                includeFile.write(os.path.abspath(value) + '\n')
            elif type(value) is list:
                for path in value:
                    includeFile.write(os.path.abspath(path) + '\n')
    if 'CPPDEFINES' in setting:
        for define in value:
            configFile.write('#define ' + define + '\n')
includeFile.close()
configFile.close()
