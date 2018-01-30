from urllib.request import urlopen
from time import sleep
import json
import random

host = "http://localhost:18080"
api_key = "api_key1"

def get_json(url):
    return json.loads(urlopen(url).read().decode('latin-1'))

def buy(ccy_pair, amount):
    url = "{}/trade/{}/buy/{}/{}".format(host, api_key, ccy_pair, amount)
    return get_json(url)

def sell(ccy_pair, amount):
    url = "{}/trade/{}/sell/{}/{}".format(host, api_key, ccy_pair, amount)
    return get_json(url)

def get_holdings():
    url = "{}/account/{}".format(host, api_key)
    return get_json(url)

# unused by this bot
def get_market():
    url = "{}/market".format(host)
    return get_json(url)

holdings = get_holdings()
while True:
    print(holdings)
    if "USD" in holdings and holdings["USD"] > 1:
        holdings = sell("USDGBP", holdings["USD"])
    else:
        holdings = sell("GBPUSD", 1000000)
    sleep(random.random())

