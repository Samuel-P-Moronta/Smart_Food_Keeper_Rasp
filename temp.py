import time
import Adafruit_DHT


SENSOR = Adafruit_DHT.DHT22
t = 0.0
h = 0.0

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


