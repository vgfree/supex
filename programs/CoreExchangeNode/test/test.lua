local socket = require("socket")

function client()
  print("client.");
  print("socket");
  host = host or "localhost"
  port = port or 8082
  if arg then
    host = arg[1] or host
    port = arg[2] or port
  end
  print("Attempting connection to host '" ..host.. "' and port " ..port.. "...")
  c = assert(socket.connect(host, port))
  print("Connected! Please type stuff (empty line to stop):")
  l = io.read()
  while l and l ~= "" and not e do
    assert(c:send(l .. "\n"))
    l = io.read()
  end
end
client();
