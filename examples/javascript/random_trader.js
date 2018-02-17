'use strict';

import {FxBattleClient} from './fxbattleclient.js';

var client = new FxBattleClient("http://localhost:8080","api_key2");

// function that handles errors returned from the api
function filterError(d){
  if(d===null)
    throw "is null";
  if(typeof(d) !== "object")
    throw "not an object";
  if(typeof(d["error"]) === "string")
    throw "error returned:" + d["error"];

  return d;
}

// function to act on an error when one is detected
function handleError(e){
  console.log("Error occured", e);
  scheduleNextTrade();
}

// schedule the next trade
function scheduleNextTrade(){
  setTimeout(makeRandomTrade, 1000);
}

// make a random trade
function makeRandomTrade() { 
  client.account().then(filterError).then(function(account) {
    console.log("account", account);
    if(account["GBP"] > 1) 
      return client.sell("GBPUSD", account["GBP"]);

    if(account["USD"] > 1) 
      return client.sell("USDGBP", account["USD"]);

    if(account["EUR"] > 1) 
      return client.sell("EURGBP", account["EUR"]);

    if(account["JPY"] > 1) 
      return client.sell("JPYGBP", account["JPY"]);

    throw "no funds";
  }).then(filterError).then(scheduleNextTrade, handleError);
}

// start the loop
makeRandomTrade();
