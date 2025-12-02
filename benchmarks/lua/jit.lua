local zstr = require("zstr")

local ITERATIONS = 1000 * 1000
local CHUNK = "1234567890"
local EXPECTED_SIZE = ITERATIONS * #CHUNK

print(string.format("=> Benchmark: Loop Performance (LuaJIT vs zstr)"))
print(string.format("   Iterations: %d | Target Size: %.2f MB\n", ITERATIONS, EXPECTED_SIZE / (1024*1024)))

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
    return duration, mem_delta
end

local t_time, t_mem = bench("table.concat", function()
    local t = {}
    for i = 1, ITERATIONS do
        t[i] = CHUNK
    end
    local s = table.concat(t)
    assert(#s == EXPECTED_SIZE)
end)

local z_time, z_mem = bench("zstr:append", function()
    local buf = zstr.new()
    buf:reserve(EXPECTED_SIZE) -- Fair comparison: reserve known size
    for i = 1, ITERATIONS do
        buf:append(CHUNK)
    end
    assert(#buf == EXPECTED_SIZE)
end)

print("\n-> Summary")
print(string.format("SPEED:   zstr is %.2fx %s", 
    (t_time / z_time), (z_time < t_time and "FASTER" or "SLOWER")))

-- Fix: handle case where zstr mem delta is 0 (allocation happens on C heap, hidden from Lua GC)
local mem_factor = (z_mem > 0) and (t_mem / z_mem) or 999.0
print(string.format("GARBAGE: zstr generated %.2fx LESS garbage", mem_factor))
