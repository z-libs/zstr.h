local zstr = require("zstr")

local ITERATIONS = 1000 * 1000
local CHUNK = "1234567890"
local EXPECTED_SIZE = ITERATIONS * #CHUNK

print(string.format("=> Benchmark: Bulk Operations"))
print(string.format("   Items: %d | Total Size: %.2f MB", ITERATIONS, EXPECTED_SIZE / (1024*1024)))

-- Pre-generate source table to exclude generation time from benchmark
print("-> Generating source table...")
local source_table = {}
for i = 1, ITERATIONS do
    source_table[i] = CHUNK
end
print("   Done.\n")

local function bench(name, func)
    collectgarbage()
    collectgarbage("stop")
    
    local start_mem = collectgarbage("count")
    local start_time = os.clock()
    
    func()
    
    local duration = os.clock() - start_time
    local mem_delta = collectgarbage("count") - start_mem
    collectgarbage("restart")
    
    print(string.format("[ %-20s ] Time: %.4fs | Mem Delta: %8.2f KB", name, duration, mem_delta))
    return duration
end

local t_time = bench("table.concat", function()
    local s = table.concat(source_table)
    assert(#s == EXPECTED_SIZE)
end)

local z_time = bench("zstr:append_table", function()
    local buf = zstr.new()
    buf:append_table(source_table)
    assert(#buf == EXPECTED_SIZE)
end)

print("\n-> Summary")
print(string.format("SPEED:   zstr is %.2fx %s than table.concat", 
    (t_time / z_time), (z_time < t_time and "FASTER" or "SLOWER")))
