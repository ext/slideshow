from setuptools import setup, find_packages

setup(
    name = "slideshow",
    version = '0.3.3',
    license = 'AGPL',
    author = 'David Sveningsson',
    author_email = 'ext@sidvind.com',

    packages = find_packages(),
    package_data = {
        '': [
            'install/*.sql',
            'lib/assembler/*.html',
            'static/*/*',
            'templates/*.html',
            'templates/*/*.html',
            'settings.xml',
            'default.xml',
            'template.dtd',
            ]},
    
    entry_points = """
[console_scripts]
slideshow = slideshow.web:run
"""
)
