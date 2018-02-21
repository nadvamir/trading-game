#!/usr/bin/env python3

"""
  Position consolidator, converts all held currency into the chosen currency

  usage position_consolidator.py --endpoint 'http://localhost:8080' --apikey 'api_key1' --currency 'JPY'
"""
import argparse
from fxbattleclient import FxClient, FxClientError
from time import sleep
import random
from math import ceil

currencies=["GBP", "EUR", "JPY", "USD"]

parser = argparse.ArgumentParser(description='Position consolidator')
parser.add_argument('--endpoint',
                    default='http://localhost:8080',
                    help='the api endpoint')
parser.add_argument('--apikey',
                    default='api_key1',
                    help='the api key')
parser.add_argument('--currency',
                    default='GBP', choices=currencies,
                    help='the selected currency')
parser.add_argument('--buffer', type=int,
                    default=10,
                    help='small buffer amount, to overcome the fee')

args = parser.parse_args()

print("Position consolidator", "endpoint:", args.endpoint, "apikey:", args.apikey, "currency:", args.currency)

client = FxClient(args.endpoint, args.apikey)
targetCurrncy = args.currency
buff = args.buffer

try:
    account = client.account()
    
    if "error" in account:
        print("could not get account details", account["error"])
        raise  

    print("account", account)      

    for cur in currencies:
      tradedPair = cur + targetCurrncy
      if cur in account and cur != targetCurrncy:
          if account[cur] > 1:
              account = client.sell(tradedPair, account[cur])
          elif account[cur] < 0:
              account = client.buy(tradedPair, ceil(-account[cur]) + buff)

    print("account", account)

except(FxClientError):
    print("error contacting the endpoint")
