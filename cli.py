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
            self._socket.send(outer_self._name)
            rc, reply = json.loads(self._socket.recv(1024))
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

    def do_EOF(self, *arg):
        sys.exit()

ssc = SlideshowCmd()
ssc.run()
