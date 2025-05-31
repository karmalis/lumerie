-- script.lua
-- Receives a table, returns the sum of its components.
local buffer = require("string.buffer")


jit.on()
io.write("JIT VERSION is: ")
io.write(jit.version)
io.write("\n")
io.write("The table the script received has:\n");
x = 0
for i = 1, #foo do
  print(i, foo[i])
  x = x + foo[i]
end
io.write("Returning data back to C\n");
return x
