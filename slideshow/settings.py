#!/usr/bin/env python
# -*- coding: utf-8 -*-

from xml.dom import minidom
import os, os.path, stat, traceback
import json, xorg_query
import pprint
import threading
import event
from lib.resolution import Resolution

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

class Item:
    typename = '' # automatically set
    
    def __init__(self, group, name, title=None, description='', rel=None, cls=''):
        self.group = group
        self.name = name
        self.title = name
        self.description = description
        self.rel = rel
        self._value = self.default
        self.cls = cls

        if title is not None:
            self.title = title
    
    def _values(self):
        return dict(
            group=self.group.name,
            name=self.name,
            title=self.title, 
            type=self.typename,
            value=unicode(self._value),
            cls=self.cls,
        )
    
    def __str__(self):
        return '<input type="text" class="{cls}" name="{group}.{name}" type="{type}" value="{value}"/>'.format(**self._values())
    
    def set(self, value, rollback=False):
        tmp = self._value
        self._value = value
        return tmp
    
    def get(self):
        return self._value

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
                raise ValueError, 'Directory not found ' + path
            
            if not stat.S_ISDIR(statinfo.st_mode):
                raise ValueError, 'Not a directory'
            if not os.access(path, os.W_OK):
                raise ValueError, 'Need write-permission'
            
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
                raise ValueError, 'Not a valid integer'
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
                raise ValueError, 'Not a valid float'
        except:
            if rollback:
                self._value = tmp
            raise

class ItemPassword(Item):
    default = ''
    
    def __str__(self):
        return '<input type="password" class="{cls}" name="{group}.{name}" type="{type}" value=""/>'.format(**self._values())

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

class ItemResolution(Item):
    default = None
    values = []
    
    def __init__(self, allow_empty=False, **kwargs):
        Item.__init__(self, **kwargs)
        self._extra = []
        
        if allow_empty:
            self._extra.append(('',''))
        
    def __str__(self):
        def f(x):
            if self._value == x:
                return '<option value="{key}" selected="selected">{value}</option>'
            else:
                return '<option value="{key}">{value}</option>'
        
        options = [f(k).format(key=k, value=v) for k,v in self._extra + self.values]
        head = '<select name="{group}.{name}" class="{cls}">'.format(**self._values())
        content = '\n'.join(options)
        tail = '</select>'
        
        return head + content + tail

class ItemDisplay(Item):
    default = ':0.0'
    values = map(lambda x: (x,x), xorg_query.screens())
    
    def __init__(self, allow_empty=False, **kwargs):
        Item.__init__(self, **kwargs)
        self._extra = []
        
        if allow_empty:
            self._extra.append(('',''))
    
    def __str__(self):
        def f(x):
            if self._value == x:
                return '<option value="{key}" selected="selected">{value}</option>'
            else:
                return '<option value="{key}">{value}</option>'
        
        options = [f(k).format(key=k, value=v) for k,v in self._extra + self.values]
        head = '<select name="{group}.{name}" class="{cls}">'.format(**self._values())
        content = '\n'.join(options)
        tail = '</select>'
        
        return head + content + tail

class ItemStatic(Item):
    default = None
    
    def set(self, rollback):
        raise RuntimeError, 'trying to set static field'

    def __str__(self):
        return '<div class="static {cls}" name="{group}.{name}">&nbsp;</div>'.format(**self._values())

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
    
    def load(self, base, config_file=None):
        doc = minidom.parse(base)
        self.groups = OrderDict()
        self.enviroment = {}
        self.config_file = config_file
        
        for group in doc.getElementsByTagName('group'):
            grpname = group.getAttribute('name')
            hidden  = group.hasAttribute('hidden') and group.getAttribute('hidden') == '1'
            ignore  = group.hasAttribute('ignore') and group.getAttribute('ignore') == '1'
            grpdesc = ''.join([x.toxml() for x in group.getElementsByTagName('description')[0].childNodes])
            
            g = Group(name=grpname, description=grpdesc, hidden=hidden, ignore=ignore)
            
            for item in group.getElementsByTagName('item'):
                attrs = {}
                for i in range(0, item.attributes.length):
                    k = item.attributes.item(i).name
                    v = item.getAttribute(k)
                    attrs[k] = v
                
                type  = attrs['type']
                del attrs['type']
                
                item = itemfactory[type](group=g, **attrs)
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
                    value = item.childNodes[0].data.format(CWD=os.getcwd())
                    try:
                        self.groups[grpname][name].set(value)
                    except:
                        traceback.print_exc()
                elif n > 1:
                    raise RuntimeError, 'Got multiple default values for %s: %s' % (item, item.childNodes)
        
        # load stored settings
        if config_file:
            self._load_user(config_file)

        # read resolution from selected display
        ItemResolution.values = resolutions(self['Appearance.Display'])

    def _load_user(self, config_file):
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
                        print 'group', k, 'found in config but is not defined xml'
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
                        except ValueError as e:
                            print name, k, 'contains illegal data ("%s": %s), resetting to default' % (value, str(e))
                        except KeyError:
                            traceback.print_exc()
                            print name, k, 'found in config but is not defined in xml'
                                
                if n == 0:
                    break
    
    def persist(self, dst=None):
        if dst is None:
            dst = self.config_file

        with open(dst, 'w') as fp:
            json.dump(self.all(), fp, indent=4)
