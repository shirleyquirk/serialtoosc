#v 0.1
#dumb router
#echos serial from serial port to a udp socket
#
#TODO: OSC client as well
#TODO: OSCQuery everything

from SlipLib.sliplib import Driver
import serial,socket
import socketserver
from time import sleep
import threading

UDP_HOST="127.0.0.1"
UDP_PORT=8888
RECV_PORT=9999
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

ser = serial.Serial('/dev/ttyUSB0',115200)
messages=[]
drv = Driver()

class MessageHandler(socketserver.BaseRequestHandler):
    def handle(self):
        data=self.request[0]
        socket=self.request[1]
        ser.write(drv.send(data))

server= socketserver.UDPServer(('',RECV_PORT),MessageHandler)
server_thread=threading.Thread(target=server.serve_forever)
server_thread.daemon=True
server_thread.start()

while True:
  sleep(1)
  #TODO: handle drv errors on improper packet
  messages += drv.receive(ser.read(ser.inWaiting()))
  for mess in messages:
    sock.sendto(mess,(UDP_HOST,UDP_PORT))
  
server.shutdown()
server.server_close()
sock.close()
