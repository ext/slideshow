#!/usr/bin/env python
# -*- coding: utf-8 -*-

from xml.dom import minidom
import os, sys, stat, traceback
import json, xorg_query
import pprint
import threading
import event
import itertools
import subprocess
from glob import glob
from lib.resolution import Resolution
from os.path import join, basename, dirname

try:
    # python 3.0
    import html.entities as entities
except ImportError:
    import htmlentitydefs  as entities

# Works like a dictionary but preserves order (well it's more like a list but
# with lookup.
class OrderDict:
    def __init__(self):
        self._list = []
        self._dict = {}

    def __setitem__(self, k, v):
        self._list.append((k,v))
        self._dict[k] = v

    def __getitem__(self, k):
        return self._dict[k]

    def values(self):
        return [v for k,v in self._list]

    def keys(self):
        return [k for k,v in self._list]

    def items(self):
        return self._list

class Group:
    def __init__(self, name, description, hidden, ignore):
        self.name = name
        self.description = description
        self.hidden = hidden
        self.ignore = ignore
        self._items = OrderDict()

    def add(self, item):
        self._items[item.name] = item

    def __getitem__(self, k):
        return self._items[k]

    def items(self):
        return self._items.items().__iter__()

    def __iter__(self):
        return self._items.values().__iter__()

class ValidationError(Exception):
    def __init__(self, msg, item, value):
        self.msg = msg
        self.item = item
        self.value = value

    def __str__(self):
        return '"%s.%s" cannot be set to "%s": %s' % (self.item.group.name, self.item.name, self.value, self.msg)

class Item:
    typename = '' # automatically set

    def __init__(self, group, name, title=None, description='', rel=None, **kwargs):
        self.group = group
        self.name = name
        self.title = name
        self.description = description
        self.rel = rel
        self._value = self.default

        self._attributes = {'name': '%s.%s' % (group.name, name)}
        self._attributes.update(kwargs)

        if title is not None:
            self.title = title

    def attribute_string(self):
        return ' '.join(['%s="%s"' % x for x in self._attributes.iteritems()])

    def _values(self):
        return dict(
            title=self.title,
            type=self.typename,
            value=unicode(self._value),
        )

    def __str__(self):
        return '<input type="text" {0} value="{value}"/>'.format(self.attribute_string(), **self._values())

    def set(self, value, rollback=False):
        tmp = self._value
        self._value = value
        return tmp

    def get(self):
        return self._value

class ItemSelect(Item):
    default = None
    values = []

    def __init__(self, allow_empty=False, **kwargs):
        Item.__init__(self, **kwargs)
        self.values = []

        if allow_empty:
            self.values.insert(0, ('',''))

    def get_options(self):
        return []

    def __str__(self):
        def f(x):
            if self._value == x:
                return '<option value="{key}" selected="selected">{value}</option>'
            else:
                return '<option value="{key}">{value}</option>'

        all = self.values + self.get_options() + self.__class__.values
        options = [f(k).format(key=k, value=v) for k,v in all]
        head = '<select {0}>'.format(self.attribute_string(), **self._values())
        content = '\n'.join(options)
        tail = '</select>'

        return head + content + tail

class ItemDummy(Item):
    default = None

class ItemDirectory(Item):
    default = ''

    def set(self, value, rollback=False):
        tmp = self._value
        self._value = value
        path = self.fullpath()

        try:
            try:
                statinfo = os.stat(path)
            except OSError:
                raise ValidationError('Directory not found ', self, path)

            if not stat.S_ISDIR(statinfo.st_mode):
                raise ValidationError('Not a directory', self, path)
            if not os.access(path, os.W_OK):
                raise ValidationError('Need write-permission', self, path)

            return tmp
        except:
            if rollback:
                self._value = tmp
            raise

    def fullpath(self):
        # quick sanity check
        if self._value == '':
            return self._value

        # is no relative path is specified it is always considered absolute
        if not self.rel:
            return self._value

        # absolute path is not relative
        if self._value[0] == '/':
            return self._value

        # join relative path with it's parent
        return os.path.join(self.rel.fullpath(), self._value)

class ItemFile(Item):
    default = ''

    def fullpath(self):
        # quick sanity check
        if self._value == '':
            return self._value

        # is no relative path is specified it is always considered absolute
        if not self.rel:
            return self._value

        # absolute path is not relative
        if self._value[0] == '/':
            return self._value

        # join relative path with it's parent
        return os.path.join(self.rel.fullpath(), self._value)

class ItemString(Item):
    default = ''

class ItemInteger(Item):
    default = 0

    def set(self, value, rollback=False):
        tmp = self._value
        self._value = value

        try:
            try:
                self._value = int(value)
                return tmp
            except ValueError:
                raise ValidationError('Not a valid integer', self, value)
        except:
            if rollback:
                self._value = tmp
            raise

class ItemFloat(Item):
    default = 0.0

    def set(self, value, rollback=False):
        tmp = self._value
        self._value = value

        try:
            try:
                self._value = float(value)
                return tmp
            except ValueError:
                raise ValidationError('Not a valid float', self, value)
        except:
            if rollback:
                self._value = tmp
            raise

class ItemPassword(Item):
    default = ''

    def __str__(self):
        return '<input type="password" {0} value="&#13;"/>'.format(self.attribute_string(), **self._values())

_aspects = [
    (4, 3),   # Regular 4:3 (VGA, PAL, SVGA, etc)
    (3, 2),   # NTSC
    (5, 3),
    (5, 4),   # SXGA, QSXGA
    (16, 10), # 16:10 "Widescreen"
    (16, 9),  # 16:9 Widescreen
    (17, 9)
]
def _calc_aspect(w,h):
    for (x,y) in _aspects:
        if (w/x) == (h/y):
            return '%d:%d' % (x,y)
    return ''

def resolutions(dpy):
    def get_key(w,h,a):
        return '{width}x{height}'.format(width=w, height=h)

    def get_value(w,h,a):
        return '{width}x{height} ({aspect})'.format(width=w, height=h, aspect=a)

    # get all available resolution, ignoring refresh-rate
    all = frozenset([(w,h,_calc_aspect(w,h)) for (w,h,r) in xorg_query.resolutions(dpy)])

    # format resolution, builing a list of ('WxH', 'WxH (A)', W, H) tuples (last elements is used for sorting)
    formated = [ (get_key(*x), get_value(*x), x[0], x[1]) for x in all]

    # sort comparision function
    def cmpfn(x,y):
        w = cmp(x[2], y[2])
        if w == 0:
            return cmp(x[3], y[3])
        return w

    # sort and drop extra element
    return [(k,v) for (k,v,w,h) in sorted(formated, cmp=cmpfn)]

class ItemResolution(ItemSelect):
    default = None
    values = [] # set later

class ItemDisplay(ItemSelect):
    default = ':0.0'
    values = map(lambda x: (x,x), xorg_query.screens())

class ItemStatic(Item):
    default = None

    def __init__(self, *args, **kwargs):
        kwargs['cls'] = kwargs.get('cls', '') + ' static'
        Item.__init__(self, *args, **kwargs)

    def __str__(self):
        return '<div {0}>&nbsp;</div>'.format(self.attribute_string(), **self._values())

class ItemFilelist(Item):
    """Select file from available choices.

    :param path: A semi-colon delimited string with filename-patterns to search
                 for. Each element is first formatted using string.format with
                 arguments 0 as setting and "root" as the slideshow root
                 directory. Next each element is globbed using glob.glob.
    """
    default = ''

    def __init__(self, path, *args, **kwargs):
        Item.__init__(self, *args, **kwargs)
        self.path = path.split(';')

    def __str__(self):
        settings = Settings()
        root = dirname(__file__)

        # glob paths
        files = [basename(x) for x in itertools.chain(*[glob(path.format(settings, root=root)) for path in self.path])]

        def f(x):
            if self._value == x:
                return '<option selected="selected">{value}</option>'
            else:
                return '<option>{value}</option>'

        options = [f(x).format(value=x) for x in files]
        head = '<select {0}>'.format(self.attribute_string(), **self._values())
        content = '\n'.join(options)
        tail = '</select>'

        return head + content + tail

class ItemTextArea(Item):
    default = ''

    def __init__(self, *args, **kwargs):
        Item.__init__(self, *args, **kwargs)

    def __str__(self):
        return '<textarea {0}>{value}</textarea>'.format(self.attribute_string(), **self._values())

class ItemCheckbox(Item):
    default = False

    def __init__(self, *args, **kwargs):
        Item.__init__(self, *args, **kwargs)

        if self._value:
            self._attributes['checked'] = 'checked'

    def set(self, value, rollback=False):
        if isinstance(value, basestring):
            value = value.lower() == 'on'
        Item.set(self, value, rollback)

    def __str__(self):
        return '<input type="checkbox" {0} />'.format(self.attribute_string(), **self._values())

class ItemTransition(ItemSelect):
    default = 'vfade'
    values = []

    def get_options(self):
        return [x.split(':', 1) for x in subprocess.check_output(['slideshow-transition', '-lb']).splitlines()]

itemfactory = {
    'directory': ItemDirectory,
    'file':      ItemFile,
    'string':    ItemString,
    'integer':   ItemInteger,
    'float':     ItemFloat,
    'password':  ItemPassword,
    'resolution':ItemResolution,
    'display':   ItemDisplay,
    'static':    ItemStatic,
    'filelist':  ItemFilelist,
    'textarea':  ItemTextArea,
    'checkbox':  ItemCheckbox,
    'transition':ItemTransition,
}

for k,v in itemfactory.items():
    v.typename = k

class Settings(object):
    __singleton = None

    def __new__(cls, *args, **kwargs):
        if cls != type(cls.__singleton):
            cls.__singleton = object.__new__(cls)
            cls.__singleton.__real_init__()
        return cls.__singleton

    def __init__(self):
        """ NOT SAFE, WILL BE CALLED FOR EACH COPY OF THE SINGLETON! """
        pass

    def __real_init__(self):
        """ Will be called on singleton creation """
        self._locked = False
        self._lock = threading.Lock()

    def __iter__(self):
        return self.groups.values().__iter__()

    def item(self, key):
        [groupname, itemname] = key.split('.')
        return self.groups[groupname][itemname]

    def __str__(self):
        pp = pprint.PrettyPrinter(indent=4)
        return pp.pformat(self.all())

    def all(self):
        c = {}
        for group in self:
            d = {}
            for item in group:
                d[item.name] = item.get()
            c[group.name] = d
        c['Env'] = self.enviroment
        return c

    def __enter__(self):
        self._lock.acquire()
        self._locked = True

    def __exit__(self, type, value, traceback):
        ItemResolution.values = resolutions(self['Appearance.Display'])

        self._locked = False
        self._lock.release()

    def __getitem__(self, key):
        if key == 'Env':
            return self.enviroment

        # full path passed, extract only the specific value
        if '.' in key:
            return self.item(key)._value

        # group name was passed, extract a dict with all values in group
        group = self.groups[key]

        d = {}
        for k,v in group.items():
            d[str(k)] = v._value

        return d

    def __setitem__(self, key, value):
        if not self._locked:
            raise RuntimeError, 'Cannot assign settings while not locked'

        if key == 'Env':
            self.enviroment = value
            return

        item = self.item(key)
        old = item.set(value)

        if key == 'Appearance.Resolution' and item._value != old:
            event.trigger('config.resolution_changed', self.resolution())

        if key == 'Runtime.queue' and item._value != old:
            event.trigger('config.queue_changed', item._value)

        if key == 'Access.Whitelist' and item._value != old:
            event.trigger('config.ipblock_enabled', item._value)

        if key == 'Access.Subnet' and item._value != old:
            event.trigger('config.ipblock_changed', item._value)

    def resolution(self):
        r = self['Appearance.Resolution']
        if r:
            w,h = r.split('x')
            w = int(w)
            h = int(h)
            return Resolution(w,h)
        else:
            size = xorg_query.current_resolution(use_rotation=True)
            return Resolution(size[0], size[1])

    def load(self, base, config_file=None, format_keys={}, errors=[]):
        """
        :param base: Path to settings XML definition file.
        :param config_file: Path to file to load settings from.
        :format_keys:
        :param errors: Decides what to do about validation errors.
          If list, elements is considered fatal and raises an error, for others a warning is issued on stderr.
          If none, all errors is ignored
        """
        doc = minidom.parse(base)
        self.groups = OrderDict()
        self.enviroment = {}
        self.config_file = config_file

        for group in doc.getElementsByTagName('group'):
            grpname = group.getAttribute('name')
            hidden  = group.hasAttribute('hidden') and group.getAttribute('hidden') == '1'
            ignore  = group.hasAttribute('ignore') and group.getAttribute('ignore') == '1'
            grpdesc = None

            # Parse group description
            grpdesc = [(''.join([x.toxml() for x in t.childNodes]), t.attributes) for t in group.getElementsByTagName('description')]

            g = Group(name=grpname, description=grpdesc, hidden=hidden, ignore=ignore)

            for item in group.getElementsByTagName('item'):
                attrs = {}
                for i in range(0, item.attributes.length):
                    k = item.attributes.item(i).name
                    v = item.getAttribute(k)
                    attrs[k] = v

                type  = attrs['type']
                del attrs['type']

                try:
                    item = itemfactory[type](group=g, **attrs)
                except KeyError, e:
                    traceback.print_exc()
                    print >> sys.stderr, 'Invalid type \"%s\" in settings.xml' % type
                    item = itemfactory['string'](group=g, **attrs)
                g.add(item)

            self.groups[grpname] = g

        # resolve relative paths
        for group in self:
            for item in group:
                if not item.rel:
                    continue

                item.rel = self.item(item.rel)

        # load default settings
        for group in doc.getElementsByTagName('group'):
            grpname = group.getAttribute('name')

            for item in group.getElementsByTagName('item'):
                name = item.getAttribute('name')

                n = len(item.childNodes)
                if n == 1:
                    # read default value from xml
                    value = item.childNodes[0].data.format(CWD=os.getcwd(), **format_keys)

                    try:
                        # Try to apply the value.
                        self.groups[grpname][name].set(value)
                    except:
                        # Failed to apply default value, this shouldn't happen.
                        # It means the default value specified in the xml is not a legal value
                        # for the datatype, eg expecting an int but got a string instead.
                        traceback.print_exc()
                elif n > 1:
                    raise RuntimeError, 'Got multiple default values for %s: %s' % (item, item.childNodes)

        # load stored settings
        if config_file:
            self._load_user(config_file, errors)

        # read resolution from selected display
        try:
            ItemResolution.values = resolutions(self['Appearance.Display'])
        except ValueError:
            traceback.print_exc()
            ItemResolution.values = []

    def _load_user(self, config_file, errors):
        with open(config_file, 'r') as f:
            c = json.load(f)

            # Because of dependencies between items and no guarantee of the
            # order they arrive in we keep iterating through all items that are
            # left until either no more changes was made or until no more items
            # are left.
            p = 0
            while True:
                p += 1 # pass
                n = 0  # nr of changes

                for k,v in c.items():
                    if k == 'Env':
                        self.enviroment = v
                        del c[k]
                        continue
                    try:
                        group = self.groups[k]
                    except KeyError:
                        print >> sys.stderr, 'group', k, 'found in config but is not defined xml'
                        continue

                    for name, value in v.items():
                        try:
                            if not group.ignore:
                                item = group[name]
                            else:
                                item = ItemDummy(group=group, name=name)
                                group.add(item)

                            item.set(value, rollback=True)
                            del v[name] # drop from list
                            n += 1
                        except ValidationError as e:
                            if errors is None:
                                continue

                            fullname ='%s.%s' % (k, name)
                            if fullname in errors:
                                raise

                            print >> sys.stderr, '"%s.%s" contains illegal data ("%s": %s), resetting to default' % (k, name, value, str(e))
                        except KeyError:
                            fullname = '%s.%s' % (k, name)
                            ignore = ['Files.BinaryPath']
                            if fullname not in ignore:
                                traceback.print_exc()
                                print >> sys.stderr, '"%s"found in config but is not defined in xml' % fullname

                if n == 0:
                    break

    def persist(self, dst=None):
        if dst is None:
            dst = self.config_file

        with open(dst, 'w') as fp:
            json.dump(self.all(), fp, indent=4)
