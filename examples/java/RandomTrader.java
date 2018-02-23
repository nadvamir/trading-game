package fxbattle;

import fxbattle.FXBattleClient;

import org.json.JSONObject;
import java.util.concurrent.ThreadLocalRandom;
import com.mashape.unirest.http.exceptions.UnirestException;

public class RandomTrader {
    public static void trade(FXBattleClient client) throws UnirestException, InterruptedException {
        JSONObject acc = client.account();

        if (acc.has("error")) {
            System.out.println(acc.getString("error"));
        }
        else if (acc.has("USD") && acc.getDouble("USD") > 1) {
            acc = client.sell("USDGBP", acc.getDouble("USD"));
        }
        else if (acc.has("GBP") && acc.getDouble("GBP") > 1) {
            acc = client.sell("GBPUSD", acc.getDouble("GBP"));
        }
        else {
            System.out.println("You're broke!");
        }

        if (acc.has("error")) {
            System.out.println("Failed to sell: " + acc.getString("error"));
        }
    }

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Usage: java -jar trader.jar http://127.0.0.1:8080 api_key");
            return;
        }
        String endpoint = args[0];
        String apiKey = args[1];
        FXBattleClient client = new FXBattleClient(endpoint, apiKey);

        while (true) {
            try {
                trade(client);
                Thread.sleep(ThreadLocalRandom.current().nextInt(100, 1000));
            }
            catch (UnirestException e) {
                System.out.println(e);
            }
            catch (InterruptedException e) {
                System.out.println(e);
            }
        }
    }
}
