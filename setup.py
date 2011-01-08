from setuptools import setup, find_packages

setup(
    name = "slideshow",
    version = '0.3.3',
    license = 'AGPL',
    author = 'David Sveningsson',
    author_email = 'ext@sidvind.com',

    packages = find_packages(),
    package_data = {
        '': ['static/*/*', 'templates/*.html', 'templates/*/*.html', 'install/*.sql', 'settings.xml']
    },
    
    entry_points = """
[console_scripts]
slideshow = slideshow.web:run
"""
)
