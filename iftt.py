import requests

key = 'XXXXXXXXXXXXXXXXXXXXXXX'

def publish_ifttt(topic, payload):
    report = {}
    channel = topic.replace('/', '%2F')
    report["value1"] = topic
    report["value2"] = payload
    requests.post("https://maker.ifttt.com/trigger/{}/with/key/{}".format(channel, key), data=report)

publish_ifttt('/domotica/salon/temperature', 19.0)
print "ok"
