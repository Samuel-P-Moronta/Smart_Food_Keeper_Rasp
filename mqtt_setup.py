import time
import paho.mqtt.client as mqttclient
import sys
from queue import Queue
weight = 0
connected = False

def on_connect (client, userdata, flags, rc):
    global connected
    if rc==0:
        #print ("cliente conectado")
        connected = True
        return connected
    else:
        #print("cliente no conectado")
        connected = False
        return connected
        
        
def on_disconnect(client, userdata, rc):
   global connected
   connected = False
   print("Retrying connection")
   setup()



def on_message (client, userdata, message):
    recibido = True
    print ("Mensaje recibido ",str (message.payload.decode("utf-8")))
    global weight 
    weight = float(str(message.payload.decode("utf-8")))

    
def getWeight():
    global weight
    peso = float(weight)
    return abs(peso)

connected = False
recibido = False
#broker = "10.0.0.10"
broker = "192.168.1.109"

port = 1883
user = "esp32"
password = "pucmm"

    

def setup():
    client = mqttclient.Client("esp32")
    client.username_pw_set(user, password = password)
    client.on_connect=on_connect
    #client.on_disconnect = on_disconnect
    client.connect(broker, port=port)
    client.subscribe("yehudy/publish")
    client.on_message = on_message
    client.loop_start()
    time.sleep(1)
    

