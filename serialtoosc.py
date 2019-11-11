#v 1.0
#dumb router
#listens for serial in
#starts udp server
#
from SlipLib.sliplib import Driver
import serial,socket
from time import sleep

UDP_HOST="127.0.0.1"
UDP_PORT=8888
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
ser = serial.Serial('/dev/ttyUSB0',115200)
messages=[]
drv = Driver()
while True:
  sleep(1)
  messages += drv.receive(ser.read(ser.inWaiting()))
  for mess in messages:
    sock.sendto(mess,(UDP_HOST,UDP_PORT))


