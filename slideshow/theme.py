import argparse
import slideshow
from os.path import join, dirname
from slideshow.settings import Settings
from slideshow.lib.assembler.text import TextAssembler, Theme
from slideshow.lib.resolution import Resolution

# arguments
parser = argparse.ArgumentParser()
parser.add_argument('-s', '--size', default=(800,600), type=lambda x: x.split('x'))
parser.add_argument('src', metavar='THEME')
parser.add_argument('dst', metavar='OUTPUT')
args = parser.parse_args()

# load stub settings
root = dirname(slideshow.__file__)
settings = Settings()
settings.load(join(root, 'settings.xml'), config_file=None, format_keys=dict(basepath=root))

# load assembler
asm = TextAssembler()
params = {
    'resolution': args.size,
}

# load theme
theme = Theme(args.src)
for item in theme.items():
    if item.name in params: continue
    params[item.name] = item.name

with open(args.dst,'w') as fp:
    asm.rasterize(Resolution(*args.size), params, file=fp, theme=args.src)
