from glob import glob
from setuptools import setup, find_packages, Extension


libconf = Extension('appconf.__appconf'
                   , sources            = ['src/appconf.cc']
                   , extra_compile_args = [ '-std=c++0x']
                   , include_dirs       = [ '/usr/local/include'
                                          , '/usr/include'
                                          , '$VIRTUAL_ENV/include' ]
                   , library_dirs       = [ '/usr/local/lib'
                                          , '/usr/lib'     
                                          , '$VIRTUAL_ENV/lib' ]
                   , libraries          = [ 'appconf' ])

setup(
    name = 'appconf',
    version = '0.0.1',
    author = 'Christopher J. Hanks',
    author_email = 'develop@cjhanks.name',
    packages     = ['appconf'],
    package_dir  = {'appconf': 'lib'},
    ext_modules  = [libconf]
)