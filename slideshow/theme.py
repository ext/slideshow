import argparse
import slideshow
from os.path import join, dirname
from slideshow.settings import Settings
from slideshow.lib.assembler.text import TextAssembler, Theme
from slideshow.lib.resolution import Resolution

# arguments
size = (800,600)
filename = 'nitroxy13.xml'

# load stub settings
root = dirname(slideshow.__file__)
settings = Settings()
settings.load(join(root, 'settings.xml'), config_file=None, format_keys=dict(basepath=root))

# load assembler
asm = TextAssembler()
params = {
    'resolution': size,
}

# load theme
theme = Theme(filename)
for item in theme.items():
    if item.name in params: continue
    params[item.name] = item.name

with open('test.png','w') as fp:
    asm.rasterize(Resolution(*size), params, file=fp, theme=filename)
