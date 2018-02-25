public class TradeBot 
{
    public static void main( String[] args ) throws InterruptedException
    {
        TradeRepository repo = new TradeRepository("http://localhost:8080", "api_key1");

        while (true) {
            // Get current holdings
            Account account = repo.Account();

            System.out.println(account);

            // If we have dollars, sell for pounds
            if (account.get("USD") != null && account.get("USD") > 1) 
                account = repo.Sell("USDGBP", account.get("USD"));

            // If we have pounds, sell for dollars
            else if (account.get("GBP") != null && account.get("GBP") > 1) 
                account = repo.Sell("GBPUSD", account.get("GBP"));

            // If we have euros, sell for pounds
            else if (account.get("EUR") != null && account.get("EUR") > 1) 
                account = repo.Sell("EURGBP", account.get("EUR"));

            // If we have yen, sell for pounds
            else if (account.get("JPY") != null && account.get("JPY") > 1) 
                account = repo.Sell("JPYGBP", account.get("JPY"));

            // Oh no!
            else
                System.out.println("You're broke!");

            Thread.sleep((long)(Math.random() * 1000));
        }

    }
}
