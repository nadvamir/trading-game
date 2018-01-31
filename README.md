# FX Battle

A not quite optimal fx exchange, providing plenty of opportunity to make money.
Write trading robots to compete against other traders! The team with most money
in the end wins.

## Main concepts

You have some initial amount of money. You can exchange it as you wish.

Every buy/sell order you submit has a trading fee, which you need to take into account.

This market is more volatile than its real world counterparts. This presents both
opportunity and danger. Choose your strategies carefully.

It is possible to ensure your transactions are making money. I can neither confirm
nor deny that this is a winning strategy. I actually can't, don't know yet.

Since we are based in the UK, your success will be measured in British pounds.

## Trading API's

### Judge the situation in the market

Request:

```
GET /market
```

Response:

```json
{
  "EURUSD": "EURUSD 1.2321 1.2323",
  "EURGBP": "EURGBP 0.7874 0.7878"
}
```

The first price is bid, the second one is ask. Bid is how much people are willing
to pay you, ask is how much people want in order to sell to you.

In the above example:

- To exchange EUR to USD (sell EURUSD), you will receive 1.2321 USD per 1 EUR you sell.

- To exchange USD to EUR (buy EURUSD), you will have to pay 1.2323 USD for each EUR you buy.

### See your account details

Request:

```
GET /account/<api_key>
```

Response:

```json
{
    "EUR": 123.23,
    "GBP": 10000.00,
    "USD": 123.34,
    "JPY": -12.0234
}
```

Returns how much of every currency you have. Can get slightly negative due to
execution quirks (assume that every account has a small overdraft).

### Trade currency

Request

```
GET /trade/<api_key>/<buy|sell>/<ccy_pair>/<amount>
```

Response: either your account details, or

```json
{
    "error": "Explanation why the trade has not been successful"
}
```

Executes the trade. For example, `/trade/api_key123/sell/GBPUSD/100.0` would
try to exchange 100 GBP for USD at the market rate.

You can specify currency pairs in whichever quoting convention is more convenient,
e.g. if GBPUSD trades at 2.0, then `sell/GBPUSD/100.0` and `buy/USDGBP/200.0`
are equivalent.

The fixed trading fee will always be applied. It is the same regardless of the
currency of your trade, e.g. 10 GBP. The fee will be converted at the market rate
and taken from the amount you are receiving. E.g. if you are selling 100 USD
and expecting to receive 10000 JPY, and 1 GBP is 200 JPY, you will receive
8000 JPY.

## Example trader

[There's a trading robot in the `example` folder.](example/random_trader.py) It shows how to use the API.
I would not recommend you to run it with your api key, since it's not the smartest
and every trade you make is live.

