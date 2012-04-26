import sys
import os
import time
import re

if len(sys.argv) < 2:
    print 'usage:', sys.argv[0], 'TAG'
    print 'where TAG can be [a-zA-Z0-9 ]'
    sys.exit(1)

tag = sys.argv[1]
root = os.path.dirname(__file__)
prefix = time.strftime('%Y%m%d%H%M%S', time.gmtime())
suffix = re.sub(r'[^a-zA-Z0-9]', '_', tag)
filename = '%s_%s.sql' % (prefix, suffix)

with open(os.path.join(root, filename), 'w') as fp:
    fp.write('-- ' + tag + '\n')
print filename, 'created'
