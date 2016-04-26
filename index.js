module.exports.client = function(port, ip, cb) {
  return __tcpClient(port, ip, cb);
};

module.exports.send = function(data) {
  return __tcpSend(data);
};