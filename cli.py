import os,sys
import socket
import cmd
import cPickle
import json

def ipc_call(func):
    def __ipc_call_wrapper(*args, **kwargs):
        rc, reply = func(*args, **kwargs)
        if rc < 0: # exception
            for line in reply:
                print line,
        else: # ok
            print reply
    return __ipc_call_wrapper

class ipc_call2:
    def __init__(self, name):
        self._name = name

    def __call__(outer_self, func):
        def __ipc_call_wrapper(self, *args, **kwargs):
            self._socket.send(json.dumps((outer_self._name,) + args))
            x = json.loads(self._socket.recv(1024))
            rc, reply = x
            if rc < 0: # exception
                for line in reply:
                    print line,
            else: # ok
                print reply
        return __ipc_call_wrapper

class Client:
    def __init__(self, file='.ipc'):
        self._socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self._socket.connect(file)

    def rpc(self, func, *args):
        call = (func,)
        print call
        print len(args)
        print args
        if len(args) > 0:
            print 'appending args'
            call += args
        print call
        call_str = json.dumps(call)
        print call_str
        self._socket.send(call_str)
        x = json.loads(self._socket.recv(1024))
        rc, reply = x
        if rc < 0: # exception
            for line in reply:
                print line,
        else: # ok
            print reply

    @ipc_call
    def list_queues(self):
        self._socket.send('get_queues')
        return json.loads(self._socket.recv(1024))

    @ipc_call
    def list_slides(self, target):
        self._socket.send('get_slides %s' % (target))
        return json.loads(self._socket.recv(1024))

    @ipc_call2('get_status')
    def get_status(self):
        pass

    @ipc_call2('start')
    def start(self):
        pass

    @ipc_call2('stop')
    def stop(self):
        pass

class SlideshowCmd(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.client = Client()

    def run(self):
        self.interactive = True
        print """bla bla bla console

Type:  '?' or 'help' for help on commands.
        """
        self.cmdloop()

    def do_exit(self, *arg, **kwargs):
        print 'arg:', arg
        print 'kwarg:', kwargs
        sys.exit()

    def do_ls(self, dst=None):
        if dst == '':
            self.client.list_queues()
        else:
            self.client.list_slides(dst)

    def do_status(self, arg):
        self.client.get_status()

    def do_start(self, args):
        print 'args', args
        self.client.rpc('start', *args)

    def do_stops(self, arg):
        self.client.stop()

    def do_EOF(self, *arg):
        sys.exit()

ssc = SlideshowCmd()
ssc.run()
