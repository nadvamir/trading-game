#!/usr/bin/env python3

"""
  Generate traders.json file

  usage generate-traders.py --currency GBP --init_amount 1000000 --output traders.json
"""
import argparse
import hashlib
import json
import random
import re
import sys

def get_api_key():
    return hashlib.sha256(bytes(random.randint(1,1000000))).hexdigest()[:6]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='traders file generator')
    parser.add_argument('--currency',
                        default='GBP',
                        help='Home currency of the traders')
    parser.add_argument('--init_amount', type=float,
                        default=1000000.0,
                        help='Starting amount of money')
    args = parser.parse_args()

    traders = {}
    while True:
        line = sys.stdin.readline()
        if line == '':
            break

        for trader in re.split(r'\s+', line):
            if trader == '':
                continue
            traders[get_api_key()] = {
                "name": trader,
                "holdings": {
                    args.currency: args.init_amount
                }
            }

    print(json.dumps(traders, indent=4))

