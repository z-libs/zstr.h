local zstr = require("zstr")

local SMALL_ITERS = 50000       -- Limit for naive (slow) concatenation
local LARGE_ITERS = 1000 * 1000 -- Limit for efficient methods
local CHUNK = "1234567890"

print(string.format("=> Benchmark: General Usage"))

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
end

-- 1. Naive Lua Concatenation (Quadratic behavior)
bench(string.format("Lua (..) %dk", SMALL_ITERS/1000), function()
    local s = ""
    for i = 1, SMALL_ITERS do
        s = s .. CHUNK
    end
end)

-- 2. table.concat (Linear behavior)
bench(string.format("table.concat %dM", LARGE_ITERS/1000000), function()
    local t = {}
    for i = 1, LARGE_ITERS do
        t[i] = CHUNK
    end
    local s = table.concat(t)
end)

-- 3. zstr (Linear behavior)
bench(string.format("zstr:append %dM", LARGE_ITERS/1000000), function()
    local buf = zstr.new()
    for i = 1, LARGE_ITERS do
        buf:append(CHUNK)
    end
end)
