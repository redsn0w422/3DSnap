__author__ = 'Eric Ahn'

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
import subprocess
import requests
import base64

api = 'http://159.203.98.104:3000/'

class SocketServer(Protocol):

    def __init__(self):
        self.buffer = ''

    def dataReceived(self, data):
        self.buffer += data

    def connectionMade(self):
        self.buffer = ''

    def connectionLost(self, reason):
        b = self.buffer.partition('\n')
        if len(b[2]) == 400 * 240 * 2 * 2:
            left, right = b[2][:len(b[2])/2], b[2][len(b[2])/2:]
            with open(b[0] + '-left.bin', 'wb') as f:
                f.write(left)
            with open(b[0] + '-right.bin', 'wb') as f:
                f.write(right)
            subprocess.call(['./convert', b[0] + '-left.bin', b[0] + '-left.png'])
            subprocess.call(['./convert', b[0] + '-right.bin', b[0] + '-right.png'])
            left_png = None
            right_png = None
            with open(b[0] + '-left.png', 'rb') as f:
                left_png = base64.b64encode(f.read())
            with open(b[0] + '-right.png', 'rb') as f:
                right_png = base64.b64encode(f.read())
            users = b[0].split(' ')
            r = requests.post(api + 'send', data={'sendTo': users[0], 'sendFrom': users[1], 'image_left': left_png, 'image_right': right_png})

class SocketServerFactory(Factory):

    protocol = SocketServer


def main():
    reactor.listenTCP(6000, SocketServerFactory())
    reactor.run()

if __name__ == '__main__':
    main()
