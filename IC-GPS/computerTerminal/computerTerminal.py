import serial
import paho.mqtt.client as mqtt
import time
import json
from datetime import datetime


def create_json_string(id, lat, lng):
    # Define the object
    data = {
        "nombre": id,
        "lat": lat,
        "lng": lng,
        # Generates current UTC time in ISO format
        "hora": datetime.utcnow().isoformat() + "Z"
    }

    # Convert the object to a JSON string
    json_string = json.dumps(data)
    return json_string


ser = serial.Serial("/dev/cu.usbmodem1301")

client = mqtt.Client()


def on_connect():
    # subscribe to any channels
    pass


def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))


#client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)

client.loop_start()

print("Starting")

while True:
    # 17:09:47.715 -> DeviceID: 1; Payload: LAT: 28.07 LONG: -15.45 || RSSI: -42
    line = ser.readline()
    print(line)

    try:
        line_str = line.decode("utf-8")

        parts = line_str.split(";")

        if (len(parts) <= 1):
            continue

        # Extract DeviceID
        device_id_part = parts[0].split(":")
        device_id = int(device_id_part[-1].strip())

    # Extract LAT, LONG, and optionally SAT from Payload
        payload_part = parts[1].split("||")[0].strip()
        payload_lat_split = payload_part.split("LAT:")
        if (len(payload_lat_split) <= 1):
            continue

        payload_long_split = payload_lat_split[1].split("LONG:")
        if (len(payload_long_split) <= 1):
            continue


        lat_long_sat_parts = payload_long_split
        device_latitude = float(lat_long_sat_parts[0].strip())

        # Check if SAT is present
        if "SAT:" in lat_long_sat_parts[1]:
            long_sat_parts = lat_long_sat_parts[1].split("SAT:")
            device_longitude = float(long_sat_parts[0].strip())
            sat_value = float(long_sat_parts[1].strip())
        else:
            device_longitude = float(lat_long_sat_parts[1].strip())
            sat_value = None
    except:
        continue

    # Print extracted values
    print("Device ID:", device_id)
    print("Device Latitude:", device_latitude)
    print("Device Longitude:", device_longitude)
    print("Device Sat number: ", sat_value)

    jsonstr = create_json_string(device_id, device_latitude, device_longitude)

    print(jsonstr)

    client.publish(
        'mimqtt/test', f'{jsonstr}')
