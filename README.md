# ml-tcp

## API

* tcpClient

``` js
  __tcpClient(
    ip,         // string
    port,       // number
    callback,   // callback
  )

  __tcpServer(
    port,       // number
    callback,   // callback
  )

  __tcpSend(
    data,       // data
  )

```

## Example

``` js
  __pinmux(35, 8);

  var receiveChannel = 'switch';
  var deviceId = 'DU8xrUWV';
  var deviceKey = 'nE1EFLIlm3TrZg79';

  var topic = 'mcs/' + deviceId + '/' + deviceKey + '/';

  __wifi({
    mode: 'station', // default is station
    auth: 'PSK_WPA2',
    ssid: 'mcs',
    password: 'mcs12345678',
  });

  global.eventStatus.on('tcpConnect', function() {
    __tcpSend('DU8xrUWV,nE1EFLIlm3TrZg79,0');

    setInterval(function() {
      __tcpSend('DU8xrUWV,nE1EFLIlm3TrZg79,0');
    }, 20000);
  });

  global.eventStatus.on('wifiConnect', function() {
    __tcpClient('54.254.183.59', 443, function(data) {
      print(data);
      var Data = data.split(',');
      //DU8xrUWV,nE1EFLIlm3TrZg79,1471930717800,switch,0

      if (Data[3] === receiveChannel) {
        __gpioWrite(35, Number(Data[4]));
      }
    });
  });

```