from flask import Flask, send_file
import requests
from PIL import Image

app = Flask(__name__)

api = 'http://159.203.98.104:3000/'

@app.route('/recv/<username>')
def recv(username):
    r = requests.post(api + 'get', data={'username': username, 'client': '3ds'})
    j = r.json()
    print(j)
    left_img_name = download_file(j['link']['link_left'])
    right_img_name = download_file(j['link']['link_right'])
    left_img = Image.open(left_img_name)
    right_img = Image.open(right_img_name)
    if left_img.mode == "RGB":
        pixelSize = 3
    else:
        pixelSize = 4
    (width, height) = left_img.size
    left_pixels = left_img.tobytes()
    left_pixels_post = ''
    for i in range(0, len(left_pixels) - 1, pixelSize):
        left_pixels_post += chr(ord(left_pixels[i+0]))
        left_pixels_post += chr(ord(left_pixels[i+1]))
        left_pixels_post += chr(ord(left_pixels[i+2]))
    right_pixels = right_img.tobytes()
    right_pixels_post = ''
    for i in range(0, len(right_pixels) - 1, pixelSize):
        right_pixels_post += chr(ord(right_pixels[i+0]))
        right_pixels_post += chr(ord(right_pixels[i+1]))
        right_pixels_post += chr(ord(right_pixels[i+2]))
    with open(username + '.bin', 'wb') as f:
        f.write(left_pixels_post)
        f.write(right_pixels_post)
    return send_file(username + '.bin')

def download_file(url):
    local_filename = url.split('/')[-1]
    r = requests.get(url, stream=True)
    with open(local_filename, 'wb') as f:
        for chunk in r.iter_content(chunk_size=1024):
            if chunk: # filter out keep-alive new chunks
                f.write(chunk)
    return local_filename

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6001, debug=True)
