import os,sys
import socket
import traceback
import threading
import json
import sqlite3

db_file = 'slideshow.db'

class Worker:
    def __init__(self):
        pass

    # call from thread, not parent
    def init(self):
        self._db = sqlite3.connect(db_file)
        self._c = self._db.cursor()

    def do_get_queues(self):
        self._c.execute('SELECT name FROM queue')
        return [x[0] for x in  self._c.fetchall()]

    def do_get_slides(self, queue):
        return [queue]

class IPCWorker(Worker):
    def __init__(self, socket):
        Worker.__init__(self)
        self._socket = socket

    def __call__(self):
        try:
            self.init()
            while True:
                line = self._socket.recv(1024)
                if len(line) == 0:
                    continue

                try:
                    reply = self.handle_line(line)

                    self._socket.send(json.dumps((0, reply)))
                except Exception, e:
                    exc_type, exc_value, exc_tb = sys.exc_info()
                    data = traceback.format_exception(exc_type, exc_value, exc_tb)
                    self._socket.send(json.dumps((-1, data)))
        except socket.error, e:
            pass
        except Exception, e:
            traceback.print_exc()

    def handle_line(self, line):
        tokens = line.split(' ')
        try:
            func = getattr(self, 'do_' + str(tokens[0]))
            return func(*tokens[1:])
        except AttributeError:
            raise NotImplementedError, '%s is not implemented' % (tokens[0])

class IPC(threading.Thread):
    def __init__(self, file='.ipc'):
        threading.Thread.__init__(self, name='IPC-Thread')

        try:
            os.remove(file)
        except:
            pass

        try:
            s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            s.bind('.ipc')
            s.listen(1)
            
            while True:
                conn, _ = s.accept()
                worker = threading.Thread(target=IPCWorker(conn))
                worker.start()

        except Exception, e:
            traceback.print_tb(None, file=sys.stdout)
            print e

        
pid = os.fork()

if pid:
    sys.exit(0)
else:
    ipc = IPC()
    ipc.start()
