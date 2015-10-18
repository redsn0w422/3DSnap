__author__ = 'Eric Ahn'

from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
import subprocess
import requests
import base64
from PIL import Image

api = 'http://159.203.98.104:3000/'

class SocketServer(Protocol):

    def __init__(self):
        self.buffer = ''

    def dataReceived(self, data):
        self.buffer += data

    def connectionMade(self):
        print 'Connected!'
        self.buffer = ''

    def connectionLost(self, reason):
        print 'Disconnected!'
        b = self.buffer.partition('\n')
        print len(b[2])
        if len(b[2]) == 400 * 240 * 2 * 3:
            left, right = b[2][:400 * 240 * 3], b[2][400 * 240 * 3:]
            #with open(b[0] + '-left.bin', 'wb') as f:
            #    f.write(left)
            #with open(b[0] + '-right.bin', 'wb') as f:
            #    f.write(right)
            # subprocess.call(['./convert', b[0] + '-left.bin', b[0] + '-left.png'])
            # subprocess.call(['./convert', b[0] + '-right.bin', b[0] + '-right.png'])
            left_image = Image.new('RGB', (400, 240))
            left_pixels = left_image.load()
            right_image = Image.new('RGB', (400, 240))
            right_pixels = right_image.load()
            for j in range(0, 240):
                for i in range(0, 400):
                    index = (400 * j + i) * 3
                    left_pixels[i, j] = (ord(left[index]), ord(left[index + 1]), ord(left[index + 2]))
                    right_pixels[i, j] = (ord(right[index]), ord(right[index + 1]), ord(right[index + 2]))
            left_image.save(b[0] + '-left.png', 'PNG')
            right_image.save(b[0] + '-right.png', 'PNG')
            left_png = None
            right_png = None
            with open(b[0] + '-left.png', 'rb') as f:
                left_png = base64.b64encode(f.read())
            with open(b[0] + '-right.png', 'rb') as f:
                right_png = base64.b64encode(f.read())
            users = b[0].split('-')
            r = requests.post(api + 'send', data={'sendTo': users[0], 'sendFrom': users[1], 'image_left': left_png, 'image_right': right_png})
            print r.text

class SocketServerFactory(Factory):

    protocol = SocketServer


def main():
    reactor.listenTCP(6000, SocketServerFactory())
    reactor.run()

if __name__ == '__main__':
    main()
