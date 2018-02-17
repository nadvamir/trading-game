'use strict';

import fetch from 'node-fetch';

export class FxBattleClient {
  constructor(endpoint, api_key){
    // the endpoint to connect to: e.g. 'http://localhost:8080'
    this.endpoint = endpoint;
    // the apikey to use: e.g. 'api_key2'
    this.api_key = api_key;
  }

  // helper function to handle the response from the api
  getJson(r) {
    if(r.status !== 200)
      throw "non 200 error code";
    if(r.headers.get('Content-Type') !== 'application/json')
      throw "unknown content type: " + r.headers.get('Content-Type');
    return r.json();
  }
  
  filterError(d) { 
    if(typeof(d) !== 'object') 
      throw "expected object";
    return d; 
  }
  
  account() {
    return fetch(this.endpoint + '/account/' + this.api_key)
      .then(this.getJson).then(this.filterError);
  }
  
  market() {
    return fetch(this.endpoint + '/market')
      .then(this.getJson).then(this.filterError);  
  }

  buy(ccy_pair, amount) {
    return fetch(this.endpoint + '/trade/' + this.api_key 
                 + '/buy/' + ccy_pair + '/' + amount )
      .then(this.getJson).then(this.filterError); 
  } 

  sell(ccy_pair, amount) {
    return fetch(this.endpoint + '/trade/' + this.api_key 
                 + '/sell/' + ccy_pair + '/' + amount )
      .then(this.getJson).then(this.filterError); 
  } 
}

