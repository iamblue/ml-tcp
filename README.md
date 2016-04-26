# ml-tcp


## API

* tcpClient

``` js
  __tcpClient(
    ip,   // string
    port, // number
    callback, // callback
  )

```

## Example

``` js
  __pinmux(35, 8);
  __tcpClient('52.77.236.179', 443, function(data) {
    // print(data);
    if (data.indexOf('switch,1') === 40) {
      __gpioWrite(35,'OUTPUT', 8, 1);
    } else {
      __gpioWrite(35,'OUTPUT', 8, 0);
    }
    // match , split ,regex not work
  });
