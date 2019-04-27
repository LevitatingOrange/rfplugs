import os
import paho.mqtt.client as mqtt
import toml
import subprocess

conf = toml.load(os.getenv("CONF_FILE", "/rflamps.toml"))

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(conf["general"]["mqtt_topic"]+"/#")

def on_message(client, userdata, msg):
    try:
        device = next(x for x in conf["devices"] if x["name"] == os.path.basename(msg.topic))
        sw = 0
        pl = msg.payload.decode()
        #print(pl)
        if pl == "ON":
            sw = 1
        elif pl == "OFF":
            sw = 0
        else:
            print("Invalid payload")
            return
        subprocess.call([conf["general"]["cmd_name"], str(device["group_id"]), str(device["device_id"]), "0", str(sw)])
    except StopIteration:
        print("Device does not exist in config!")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(conf["general"]["username"], conf["general"]["password"])
client.connect(conf["general"]["server"], int(conf["general"]["port"]), 60)
print("Starting...")
client.loop_forever()
