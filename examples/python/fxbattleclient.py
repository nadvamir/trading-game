from urllib.request import urlopen
from urllib.error import URLError
import json

class FxClientError(Exception):
    def __init__(self, message='Generic error'):
      self.message = message
  

class FxClient:
  """
    FxClient(endpoint, api_key)

    args:
      endpoint: the full http url to the exchange incl port. no ending slash
                e.g. 'https://127.0.0.1:8080'
      api_key : the key
                e.g. 'api_key1'
    raises:
      connection errors alone will be raised as FxClientError 
    
      all failures returned by the api will be returned
        e.g. res = client.buy("GBPUSD", -1)
      will _not_ raise FxClientError
      but ("error" in res) == True
    
  """

  def __init__(self, endpoint, api_key):
    self._endpoint = endpoint
    self._api_key = api_key

  #private
  def _get_json(self, url):
    try:
      return json.loads(urlopen(url).read().decode('latin-1'))
    except(URLError):
      raise FxClientError()

  #public interface
  def buy(self, ccy_pair, amount):
    url = "{}/trade/{}/buy/{}/{}".format(self._endpoint, self._api_key, ccy_pair, amount)
    return self._get_json(url)

  def sell(self, ccy_pair, amount):
    url = "{}/trade/{}/sell/{}/{}".format(self._endpoint, self._api_key, ccy_pair, amount)
    return self._get_json(url)

  def account(self):
    url = "{}/account/{}".format(self._endpoint, self._api_key)
    return self._get_json(url)

  def market(self):
    url = "{}/market".format(self._endpoint)
    return self._get_json(url)

