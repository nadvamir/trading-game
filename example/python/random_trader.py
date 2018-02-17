#!/usr/bin/env python3

"""
  Example random trader

  usage random_trader.py --endpoint 'http://localhost:8080' --apikey 'api_key1' --timeout '1000'
"""
import argparse
from fxbattleclient import FxClient, FxClientError
from time import sleep
import random

parser = argparse.ArgumentParser(description='Example random trader')
parser.add_argument('--endpoint',
                    default='http://localhost:8080',
                    help='the api endpoint')
parser.add_argument('--apikey',
                    default='api_key1',
                    help='the api key')
parser.add_argument('--timeout', type=int,
                    default=1000,
                    help='affects the speed of the trader')

args = parser.parse_args()

print("Random Trader", "endpoint:", args.endpoint, "apikey:", args.apikey)
client = FxClient(args.endpoint, args.apikey)
timeout = args.timeout

try:
  while True:
      account = client.account()
      
      if "error" in account:
          print("could not get account details", account["error"])  
          continue
  
      print("account", account)      

      if "USD" in account and account["USD"] > 1:
          account = client.sell("USDGBP", account["USD"])
      elif "GBP" in account and account["GBP"] > 1:
          account = client.sell("GBPUSD", account["GBP"])
      elif "EUR" in account and account["EUR"] > 1:
          account = client.sell("EURGBP", account["EUR"])
      elif "JPY" in account and account["JPY"] > 1:
          account = client.sell("JPYGBP", account["JPY"])
      else:
          print("you're broke!")

      if "error" in account:
          print("failed to sell", account["error"])
      
      sleep(0.1 + ((timeout/1000.0) * random.random()))

except(FxClientError):
    print("error contacting the endpoint")
except(KeyboardInterrupt):
    print()
    print("final holdings", client.account())
