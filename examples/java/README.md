A java library for trading on the exchange.

## Documentation

Construct a trading repo with a URL, and API key:

```java
TradeRepository repo = new TradeRepository("http://server:8080", "api_key1");
```

The trading repo has four methods, which mirror the four server API calls:

```java
// Retrieve your current account balances
Account currentBalance = repo.Account();
System.out.printf("I currently have %f USD\n", currentBalance.get("USD"));

// Access the market rates of various currencies
Market marketRates = repo.Market();
System.out.printf("The market rate for GBPUSD is currently %f\n", marketRates.get("GBPUSD"));

// Buy and sell currency pairs
Account updatedAccount = repo.Sell("GBPUSD", 10000.0);
updatedAccount = repo.Buy("JPYGBP", 10000.0);
```

## Example

An example (if extremely dumb) trader is included in [TradeBot.java](./src/main/java/tradebot/TradeBot.java), which can be run using `mvn compile exec:java`.