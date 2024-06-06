local smv      = require("smv")
local plot     = require("plot")
local pl3d     = require("pl3d")
local json     = require("json")
local lfs      = require("lfs")
local affinity = require("affinity")
local case     = smvlib.load_default()

-- We pretty much always want to output the HRR so we will do that first.
local hrrDV = case.csvs['hrr'].vectors['HRR']
plot.plotHRRDV(plotDir, hrrDV, string.format("HRR %s", case.chid),
    { xrange = "[0:" .. tend .. "]", yrange = "[0:" .. maxHRR .. "]" },
    {
        bounds = {
            capTime = 300, -- Peak occurs at 300s
            bound = 0.1, -- 10%
            growthRate = "medium",
        },
        door = { open = 300, close = 320 }
    })
