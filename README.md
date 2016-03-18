# ml-tcp


## API

* tcpClient

``` js
  tcpClient(
    ip,   // string
    port, // number
    callback, // callback
  )

```

## Example

``` js

  tcpClient('52.77.236.179', 443, function(data) {
    // print(data);
    if (data.indexOf('switch,1') === 40) {
      gpio(35,'OUTPUT', 8, 1);
    } else {
      gpio(35,'OUTPUT', 8, 0);
    }
    // match , split ,regex not work
  });
