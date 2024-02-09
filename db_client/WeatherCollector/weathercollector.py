#!/usr/bin/python3 -u
# This is a daemon which receives messages from mqtt and writes them to our wetter-Datenbank.
# install mysql-connector-python!!! 8.3.0 or later
import mysql.connector
# install paho.mqtt 1.6.1 or later
import paho.mqtt.client as mqtt
import numbers
import configparser

mq_host = ''
mq_topic = ''
db_user = ''
db_pass = ''
db_name = ''
db_host = ''


def on_message(client, userdata, message):
    #print("message topic=",message.topic)
    #print("message received ", str(message.payload.decode("utf-8")))
    value =  message.payload.decode("utf-8")
    try:
      num = float(value) 
      sql = "insert into wetter (domain, val, date_time) values ('" + message.topic + "'," + value + ", NOW() )"
      #cnx.cmd_query(sql)
      #cnx.commit()
      print(sql)
    except:
      #print("something went wrong")
      pass
      

def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f' {name}')  # Press Strg+F8 to toggle the breakpoint.

def init_config():
    global db_user, db_pass, db_name, db_host, mq_host, mq_topic
    config = configparser.ConfigParser()
    config.read('config.ini')

    db_user = config['database']['username']
    db_pass = config['database']['password']
    db_name = config['database']['database']
    db_host = config['database']['host']

    mq_host = config['MQTT']['host']
    mq_topic = config['MQTT']['topic']


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    #print_hi('starting up...')
    init_config()
    cnx = mysql.connector.connect(user=db_user, password=db_pass,
                              host=db_host,
                              database=db_name)

    client = mqtt.Client("wetter")
    client.connect(mq_host)
    client.subscribe(mq_topic)
    client.on_message = on_message

    run = True
    while run:
        client.loop()

    cnx.close()



