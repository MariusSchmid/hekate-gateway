

import json
import random
import socket
import datetime

protocol_version = 2
push = 0
mac = 0xA84041FDFEEFBE63

mac_bytes = mac.to_bytes(8, 'big') 

header =[
        protocol_version, 
        random.randint(0,255), 
        random.randint(0,255), 
        push
        ]

header.extend(mac_bytes)

date =  "2024-07-15 00:00:00 UTC"
# date = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")
status_string =  '{"stat":{"time":"","rxnb":1,"rxok":1,"rxfw":1,"ackr":100.0,"dwnb":0,"txnb":0}}'

status_json = json.loads(status_string)
status_json['stat']['time'] = date

status = json.dumps(status_json)

message = bytearray(header)
message.extend(status.encode())

print(bytes(message))



# UDP_IP = "eu1.cloud.thethings.network"
# UDP_IP = "192.168.0.113"
UDP_IP = "52.212.223.226"
UDP_PORT = 1700

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.sendto(bytes(message), (UDP_IP, UDP_PORT))