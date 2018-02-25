import com.mashape.unirest.http.HttpResponse;
import com.mashape.unirest.http.JsonNode;
import com.mashape.unirest.http.Unirest;
import com.mashape.unirest.http.exceptions.UnirestException;

import org.json.JSONObject;
import java.util.Map;
import java.util.Iterator;


public class TradeRepository
{
    private String _endpoint;
    private String _apiKey;

    public TradeRepository(String givenEndpoint, String givenApiKey) 
    {
        this._endpoint = givenEndpoint;
        this._apiKey = givenApiKey;
    }

    public Account Buy(String pair, double quantity)
    {
        System.out.println("Buying " + quantity + " of " + pair);
        return makeTrade(pair, quantity, false);
    }

    public Account Sell(String pair, double quantity)
    {
        System.out.println("Selling " + quantity + " of " + pair);
        return makeTrade(pair, quantity, true);
    }

    public Account Account()
    {
        String url = String.format("%s/account/%s", _endpoint, _apiKey);
        return deserializeAccount(getJson(url));
    }

    public Market Market()
    {
        String url = String.format("%s/market", _endpoint);
        JSONObject response = getJson(url);

        Iterator<String> iterator = response.keys();

        Market toReturn = new Market();

        while (iterator.hasNext())
        {
            String currencyPair = iterator.next();
            String[] currencyPairDetails = response.getString(currencyPair).split(" ");
            Double bid = Double.parseDouble(currencyPairDetails[1]);
            Double ask = Double.parseDouble(currencyPairDetails[2]);
            toReturn.Pairs.put(currencyPairDetails[0], new CurrencyPair(bid, ask));
        }

        return toReturn;
    }

    private Account makeTrade(String pair, double quantity, boolean sell)
    {
        String url = String.format("%s/trade/%s/%s/%s/%s", _endpoint, _apiKey, sell ? "sell" : "buy" ,pair, quantity);
        return deserializeAccount(getJson(url));
    }

    private Account deserializeAccount(JSONObject givenObject)
    {
        if (givenObject.has("error"))
        {
            throw new TradingException(givenObject.getString("error"));
        }

        Iterator<String> keys = givenObject.keys();

        Account toReturn = new Account();
        while (keys.hasNext()) {
            String key = keys.next();
            toReturn.put(key, givenObject.getDouble(key));
        }

        return toReturn;
    }
    
    private JSONObject getJson(String givenUrl) {
        try 
        {
            return Unirest.get(givenUrl).asJson().getBody().getObject();
        }
        catch (UnirestException e) 
        {
            throw new TradingException(e);
        }

    }
}