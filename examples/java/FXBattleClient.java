package fxbattle;

import com.mashape.unirest.http.*;
import com.mashape.unirest.http.exceptions.UnirestException;
import org.json.JSONObject;

public class FXBattleClient {
    private String endpoint;
    private String apiKey;

    public FXBattleClient(String endpoint, String apiKey) {
        this.endpoint = endpoint;
        this.apiKey = apiKey;
    }

    public JSONObject buy(String ccyPair, Double amount) throws UnirestException {
        return Unirest
            .get(this.endpoint + "/trade/{api_key}/buy/{ccy_pair}/{amount}")
            .routeParam("api_key", this.apiKey)
            .routeParam("ccy_pair", ccyPair)
            .routeParam("amount", amount.toString())
            .asJson().getBody().getObject();
    }

    public JSONObject sell(String ccyPair, Double amount) throws UnirestException {
        return Unirest
            .get(this.endpoint + "/trade/{api_key}/sell/{ccy_pair}/{amount}")
            .routeParam("api_key", this.apiKey)
            .routeParam("ccy_pair", ccyPair)
            .routeParam("amount", amount.toString())
            .asJson().getBody().getObject();
    }

    public JSONObject account() throws UnirestException {
        return Unirest
            .get(this.endpoint + "/account/{api_key}")
            .routeParam("api_key", this.apiKey)
            .asJson().getBody().getObject();
    }

    public JSONObject market() throws UnirestException {
        return Unirest
            .get(this.endpoint + "/market")
            .asJson().getBody().getObject();
    }
}

