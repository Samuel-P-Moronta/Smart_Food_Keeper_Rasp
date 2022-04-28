import websocket
import _thread
from datetime import datetime, time
import time
import json, random
from threading import Thread
import sys
sys.path.append('/home/pi/Desktop/Testing/Smart-Food-Keeper')
import temp
import mqtt_setup as mqtt

import socket
import websocket
import queue
import threading
import traceback
import ssl 


#TEMPERATURA

import Adafruit_DHT


SENSOR = Adafruit_DHT.DHT22
t = 0.0
h = 0.0
ipbk= "sfkproject.tech"
#ipbk = "10.0.0.12"
def showTemp():
    
    humedad, temperatura = Adafruit_DHT.read(SENSOR, 4)
    #h, t = Adafruit_DHT.read(SENSOR, 4)
    #print("Temp={0:0.1f}C Hum={1:0.1f}%".format(temperatura,humedad))
    global t
    global h
    if humedad is not None and temperatura is not None:    
        t = round (temperatura,2)
        h = round (humedad,2)
    return t, h
    

        


def readTempHum():
    temp, hum = showTemp()
    return temp, hum







#++++++++++++++++++++++++++++++++++++++
#                                      #
# SMART FOOD KEEPER-FINAL PROJECT BY:  #
# SAMUEL P. MORONTA | YEHUDY DE PEÑA   #
#                                      #
#++++++++++++++++++++++++++++++++++++++#

HEADER = 64
PORT = 6000
SERVER = "192.168.1.109"
#SERVER = "192.168.0.8"
ADDR = (SERVER, PORT)
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)




def on_message(ws, message):
    print(message)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")
    print ("### attempting reconnection ###")
    time.sleep(5)
    connect_websocket_shelf()

def on_close_container(ws, close_status_code, close_msg):
    print("### closed ###")
    print ("### attempting reconnection to container###")
    time.sleep(5)
    connect_websocket_container()
    




def send_sensor_data(ws):
    def run(*args):
        #temperature = round(random.uniform(30.00, 31.00), 2)
        #humidity = round(random.uniform(60.00, 61.00), 2)
        server.listen()
        
        conn, addr = server.accept()
        print(f"[NEW CONNECTION] {addr} connected.")
        i=0
        try:
            while True:
                
                temperature , humidity = showTemp()
                print("[GETTING] Getting info from ML client")
                msg_length = conn.recv(HEADER).decode(FORMAT)
                if msg_length:
                    msg_length = int(msg_length)
                    # Recibo todas las informaciones del reconocimiento
                    msg = conn.recv(msg_length).decode(FORMAT)
                    
                    json_object = json.loads(msg)
                    fruit_cant = json_object["fruitCant"]
                    fruit_type = json_object["fruitType"]
                    cant_overripe = json_object["cantOverripe"]
                    cant_ripe = json_object["cantRipe"]
                    cant_unripe = json_object["cantUnripe"]
                    deviceId = "1"

                    reg_data = {
                        "temperature":temperature,
                        "humidity": humidity,
                        "fruitCant": fruit_cant,
                        "fruitType": fruit_type,
                        "cantOverripe": cant_overripe,
                        "cantRipe": cant_ripe,
                        "cantUnripe": cant_unripe,
                        "deviceId": deviceId,
                    }
                    json_reg_data = json.dumps(reg_data, indent=4, default=str)
                    #if (i == 30):
                    #    ws.send(json_reg_data)
                    #    i=0
                    i = i + 1
                    if i == 2:
                        print("[DATO ENVIADO]")
                        ws.send(json_reg_data)
                        i = 0
                    

            conn.close()
            ws.close()
            print("thread terminating...")

        except Exception:
            print(traceback.format_exc())

    _thread.start_new_thread(run, ())
    
    
def handle_client_ML(conn, addr, que_msg):
    print(f"[NEW CONNECTION] {addr} connected.")

    connected = True
    while connected:
        msg_length = conn.recv(HEADER).decode(FORMAT)
        if msg_length:
            msg_length = int(msg_length)
            msg = conn.recv(msg_length).decode(FORMAT)
            # Recibo todas las informaciones del reconocimiento
            que_msg.put(msg)

    conn.close()





def connect_websocket_shelf():
    """
    Function to connect websocket shelf endponit /server/shelf
    """

    websocket.enableTrace(True)
    # Change localhost to your current ip localhost example: 10.0.0.10
    ws = websocket.WebSocketApp("wss://"+ipbk+"/server/shelf",
                                on_open=send_sensor_data,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})

def connect_websocket_container():
    """
    Function to connect websocket container endponit /server/container
    """

    websocket.enableTrace(True)
    # Change localhost to your current ip localhost example: 10.0.0.10
    ws = websocket.WebSocketApp("wss://"+ipbk+"/server/container",
                                on_open=send_container_data,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close_container)

    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})
    
    

def simulate_realtime_ml_data_1(ws):
    def run(*args):
        while True:
            temperature = round(random.uniform(25.00, 38.00), 2)
            humidity = round(random.uniform(50.00, 60.00), 2)
            overripe = 2
            ripe = 1
            unripe = 1
            fruitCant = 4

            random_reg_data = {
                "deviceId": "2",
                "temperature": temperature,
                "humidity": humidity,
                "fruitCant": fruitCant,
                "fruitType": "papaya",
                "cantOverripe": overripe,
                "cantRipe": ripe,
                "cantUnripe": unripe
            }
            json_reg_data = json.dumps(random_reg_data, indent=4, default=str)
            time.sleep(10)
            ws.send(json_reg_data)
        ws.close()
    _thread.start_new_thread(run, ())


def simulate_realtime_ml_data_2(ws):
    def run(*args):
        while True:
            temperature = round(random.uniform(25.00, 38.00), 2)
            humidity = round(random.uniform(50.00, 60.00), 2)
            overripe = 0
            ripe = 2
            unripe = 1
            fruitCant = 3

            random_reg_data = {
                "deviceId": "3",
                "temperature": temperature,
                "humidity": humidity,
                "fruitCant": fruitCant,
                "fruitType": "pineapple",
                "cantOverripe": overripe,
                "cantRipe": ripe,
                "cantUnripe": unripe
            }
            json_reg_data = json.dumps(random_reg_data, indent=4, default=str)
            time.sleep(10)
            ws.send(json_reg_data)
        ws.close()
    _thread.start_new_thread(run, ())


def simulate_realtime_ml_data_3(ws):
    def run(*args):
        while True:
            temperature = round(random.uniform(30.00, 40.00), 2)
            humidity = round(random.uniform(55.00, 70.00), 2)
            overripe = 1
            ripe = 2
            unripe = 2
            fruitCant = 4

            random_reg_data = {
                "deviceId": "4",
                "temperature": temperature,
                "humidity": humidity,
                "fruitCant": fruitCant,
                "fruitType": "papaya",
                "cantOverripe": overripe,
                "cantRipe": ripe,
                "cantUnripe": unripe
            }
            json_reg_data = json.dumps(random_reg_data, indent=4, default=str)
            time.sleep(10)
            ws.send(json_reg_data)
        ws.close()
    _thread.start_new_thread(run, ())


def send_container_data(ws):
    def run(*args):
        i = 0
        counter = 0
        saved = 0
        while True:
            if (mqtt.getWeight() < (saved + 0.2) and mqtt.getWeight() > (saved - 0.2)): #evalua que el nuevo dato es igual al guardado
                counter = counter + 1
            else: #si el dato es diferente se guarda el nuevo valor y se empieza el conteo nuevamente
                saved = mqtt.getWeight()
                counter = 0
            weight_data_in = saved
            if (abs(weight_data_in) == 0.1 or abs(weight_data_in)==0.2 or abs(weight_data_in)==0.3):
                weight_data_in=0
            container_data = {
                'containerId': "1",
                'weight': weight_data_in
            }
            # websocket.
            if (counter == 25):
                time.sleep(1)               

                print("[*] Sending data # [{}] to container ".format(i))
                json_sensor_data = json.dumps(container_data, indent=4, default=str)
                ws.send(json_sensor_data)
                print("+++++++++++++++++++++++++++++++++++")
                i = i + 1

        # time.sleep(1)
        #ws.close()
        print("thread terminating...")

    _thread.start_new_thread(run, ())


def connect_websocket_shelf_simulate_1():
    """
    Function to connect websocket shelf endponit /server/shelf
    """
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://"+ipbk+"/server/shelf", on_open=simulate_realtime_ml_data_1, on_message=on_message,
                                on_error=on_error, on_close=on_close)
    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})


def reconnect():
    connect_websocket_shelf()
    connect_websocket_container()
    
def connect_mqtt():
    mqtt.setup()
    
if __name__ == "__main__":
    "Threading to keep twice function running"
    Thread(target=connect_websocket_shelf).start()
    Thread(target=connect_websocket_container).start()
    Thread(target=connect_mqtt).start()
    #Thread(target=connect_websocket_container).start()
    #Thread(target=connect_websocket_shelf_simulate_1).start()

