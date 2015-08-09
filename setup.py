from setuptools import setup, find_packages

DESCRIPTION = '''\
Bob is a suite of implementations of the Scheme language in Python'''

setup(
    name='bobscheme',
    description=DESCRIPTION,
    author="Eli Bendersky",
    author_email="eliben@gmail.com",
    version='1.0.0',
    packages=find_packages(),
    license='PD',
    entry_points={
        "console_scripts": [
            'bob = bob.cmd:main',
        ],
    },
)
