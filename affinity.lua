--- @module 'plot'
local affinity = {}

local plot = require("plot")
local json = require("json")
local lfs = require("lfs")


function affinity.meshInfoTable(case)
    local s             = ""
    local totalCells    = 0
    local maxNameLength = 0
    local maxIJKLength  = 0
    for k, mesh in ipairs(case.meshes) do
        if #mesh.label > maxNameLength then
            maxNameLength = #mesh.label
        end
        local ijkLength = string.format("%d-%d-%d", mesh.ibar, mesh.jbar, mesh.kbar)
        if #ijkLength > maxIJKLength then
            maxIJKLength = #ijkLength
        end
    end
    s = s .. string.format("CHID: %s", case.chid) .. "\n"
    s = s .. string.format("----------------------------------------------------------------") .. "\n"
    s = s .. string.format("#\t|\t     MeshId.\t|\t# Cells\t| I-J-K") .. "\n"
    s = s .. string.format("----------------------------------------------------------------") .. "\n"
    for k, mesh in ipairs(case.meshes) do
        local nCells = mesh.ibar * mesh.jbar * mesh.kbar
        totalCells = totalCells + nCells
        local s = string.format("%%d\t| %%%ds\t|\t%%d\t| %%%ds", maxNameLength, maxIJKLength)
        s = s .. string.format(s, k, mesh.label, nCells,
            string.format("%d-%d-%d", mesh.ibar, mesh.jbar, mesh.kbar)) .. "\n"
    end
    s = s .. string.format("----------------------------------------------------------------") .. "\n"
    s = s .. string.format("Total\t|\t        \t|\t%d\t|", totalCells) .. "\n"
    s = s .. string.format("----------------------------------------------------------------") .. "\n"
end

-- The following commands set standard parameters for Affinity's house style.
function affinity.setHouseStyle(smv)
    -- Set the color bar to a blue/red split.
    smv.view.colorbar.preset = "blue->red split"
    -- Set the window size to 800×500
    smv.view.window:set_size(800, 500)
    -- Don't show the title of the model
    smv.view.show_title = false
    -- Don't show the version of smokeview being used.
    smv.view.show_smv_version = false
    -- Show the CHID of the model
    smv.view.show_chid = true
    -- Don't show blockages
    smv.view.blockages.hide_all()
    -- Don't show any surfaces
    smv.view.surfaces.hide_all()
    -- Don't show any devices
    smv.view.devices.hide_all()
    -- smv.view.outline = false
    -- smv.view.outline.hide()
    -- smv.view.font.size = "large"
    smv.view.font.size = "large"

    -- BEGIN: Setup output
    smv.render.dir = "renders"
    smv.render.type = "PNG"
    -- END: Setup output
end

---@alias plotOpts { title: string, name: string, dir: string, yName: string, yUnits: string, gnuplot: table, averaged: number, retain: string, extraVecs: table, transform: string, averagedOnly: boolean}

--- Plot the sum of a set of devc vectors defined by a given pattern.
--- @param case table The case to plot from
--- @param pattern string | function Any vector names which match this pattern will be
---  plotted.
--- @param opts plotOpts Various options to control the output.
function affinity.plotDevcs(case, pattern, opts)
    local vecs = {};
    if not opts then
        opts = {}
    end
    if type(pattern) == "string" then
        for k, v in pairs(case.csvs['devc'].vectors) do
            if k:match(pattern) then
                vecs[#vecs + 1] = v
            end
        end
    elseif type(pattern) == "function" then
        for k, v in pairs(case.csvs['devc'].vectors) do
            if pattern(k, v) then
                vecs[#vecs + 1] = v
            end
        end
    end
    -- If primaryVec has a value, we are plotting a "single" series (albeit perhaps
    -- with some additional series for information) as opposed to a group of
    -- series.
    local primaryVec
    if #vecs > 0 then
        local plotVecs
        if opts.transform == "sum" then
            local vec
            for i, v in ipairs(vecs) do
                if vec then
                    vec = plot.addDVs({ vec, v })
                else
                    vec = v
                end
            end
            plotVecs = { vec }
        else
            plotVecs = vecs
        end
        -- Many of these actions can only be applied to a single series
        if plotVecs and #plotVecs == 1 then
            primaryVec = plotVecs[1]
            if opts.yName then
                primaryVec.y.name = opts.yName
            end
            if opts.name then
                primaryVec.name = opts.name
            end
            if opts.yUnits then
                primaryVec.y.units = opts.yUnits
            end
            if not opts.title then
                opts.title = primaryVec.y.name
            end
            if opts.averaged then
                local averaged = plot.wma(plot.copyDV(primaryVec), opts.averaged)
                averaged.name = primaryVec.y.name .. " (Time-Averaged)"
                if opts.averagedOnly then
                    plotVecs[1] = averaged
                end
            end
            if opts.retain then
                local s = json.encode(primaryVec)
                local f = assert(io.open(opts.retain, "w"))
                f:write(s)
                f:close()
            end
        end
        if not opts.dir then
            opts.dir = "."
        end
        if not opts.gnuplot then
            opts.gnuplot = {}
        end
        if opts.extraVecs then
            for i, v in ipairs(opts.extraVecs) do
                plotVecs[#plotVecs + 1] = v
            end
        end
        plot.DV(opts.dir, plotVecs,
            opts.title, opts.gnuplot)
        return primaryVec
    end
end

function affinity.plotFlows(case, potential, title, opts)
    if not opts then
        opts = {}
    end
    if not opts.avgWindow and opts.avgWindow ~= 0 then
        -- The default avergae window is 40 'frames'
        opts.avgWindow = 40
    end
    if not opts.dir then
        opts.dir = "."
    end
    local vFlows = {}
    local sFlows = {}
    for i, item in ipairs(potential) do
        local vs = {}
        local varName = item[1]
        local showName = item[2]
        if not showName then
            showName = varName
        end
        local vFlow = case.csvs['devc'].vectors[varName .. 'VFlow']
        local sFlow = case.csvs['devc'].vectors[varName .. 'SFlow']
        -- TODO: we don't know whether to negate or not.
        if vFlow then
            vFlow = plot.wma(vFlow, opts.avgWindow)
            vFlow.y.name = "Volume Flow"
            vFlow.name = showName .. " Volume Flow"
            vs[#vs + 1] = vFlow
            vFlows[#vFlows + 1] = vFlow
        end
        if sFlow then
            sFlow = plot.wma(sFlow, opts.avgWindow)
            sFlow.y.name = "Soot Mass Flow"
            sFlow.name = showName .. " Soot Mass Flow"
            vs[#vs + 1] = sFlow
            sFlows[#sFlows + 1] = sFlow
        end
        if #vs > 0 then
            plot.DV(opts.dir, vs,
                showName, opts.gnuplot)
        end
    end
    if #vFlows > 0 then
        plot.DV(opts.dir, vFlows,
            title .. " Volume Flows", opts.gnuplot)
    end
    if #sFlows > 0 then
        plot.DV(opts.dir, sFlows,
            title .. " Soot Mass Flows", opts.gnuplot)
    end
end

local function withDir(dir, f, ...)
    local pwd = lfs.currentdir()
    lfs.chdir(dir)
    local res = pcall(f, ...)
    lfs.chdir(pwd)
    return res
end

function affinity.getInfo(case)
    local path = "."
    local tempDir = path .. "/" .. "temp"
    lfs.mkdir(tempDir)
    local inpath = string.format("%s.fds", case.chid)
    print(lfs.currentdir())
    print("inpath:", inpath)
    local infile, inerr = io.open(inpath, "r")
    print(infile)
    if not infile then
        error("no infile: " .. inerr)
    end
    local instr = infile:read("*a")
    infile:close()
    local outpath = string.format(tempDir .. "/%s.fds", case.chid)
    local outfile, outerr = io.open(outpath, "w")
    if not outfile then
        error("no outfile: " .. outerr)
    end
    outfile:write(instr)
    outfile:close()
    local info
    local code, val = withDir(tempDir, function()
        local cmd = string.format("fds-verify \"%s.fds\" --json -", case.chid)
        local handle, err = io.popen(cmd, "r")
        if not handle then
            error(err)
        end
        local result = handle:read("*a")
        handle:close()
        info = json.decode(result)
        return info
    end)
    for file in lfs.dir(tempDir) do
        if file ~= "." and file ~= ".." then
            local f = tempDir .. '/' .. file
            print("\t " .. f)
            os.remove(f)
        end
    end
    lfs.rmdir(tempDir)
    return info
end

local function FlowRate(info, vent)
    local surface
    for i, surf in ipairs(info.surfaces) do
        if surf.id == vent.surface then
            surface = surf
            break
        end
    end
    if (not surface) then
        return
    end
    return surface.volume_flow;
end

local function CommonPrefix(string1, string2)
    local maxLen = string1:len()
    if string2:len() > maxLen then
        maxLen = string2:len()
    end
    local prefixLen = 0
    for i = 1, maxLen do
        if string1:sub(i, i) == string2:sub(i, i) then
            prefixLen = prefixLen + 1
        else
            return string1:sub(1, prefixLen)
        end
    end
end

local function TrimPunctuationEnd(s)
    local r = s:len()
    while s:sub(r, r) == ' ' or s:sub(r, r) == '_' or s:sub(r, r) == '-' or s:sub(r, r) == '.' do
        r = r - 1
    end
    return s:sub(1, r)
end

local function IsFlowDevice(device)
    if (device.quantities[1] == "U-VELOCITY" or device.quantities[1] == "V-VELOCITY" or device.quantities[1] == "W-VELOCITY") and device.spatial_statistic == "AREA INTEGRAL" then
        return true
    end
    if device.quantities[1] == "NORMAL VELOCITY" and device.spatial_statistic == "SURFACE INTEGRAL" then
        return true
    end
    if device.quantities[1] == "VOLUME FLOW" then
        return true
    end
    return false
end

local function IsSootFlowDevice(device)
    if (device.quantities[1] == "TOTAL MASS FLUX X" or device.quantities[1] == "TOTAL MASS FLUX Y" or device.quantities[1] == "TOTAL MASS FLUX Z") and device.spatial_statistic == "AREA INTEGRAL" and device.spec_id == "SOOT" then
        return true
    end
    if device.quantities[1] == "MASS FLUX WALL" and device.spatial_statistic == "SURFACE INTEGRAL" and device.spec_id == "SOOT" then
        return true
    end
    if (device.quantities[1] == "MASS FLOW") and device.spec_id == "SOOT" then
        return true
    end
    return false
end

local function DimensionsMatch(a, b)
    return a.x1 == b.x1 and
        a.x2 == b.x2 and
        a.y1 == b.y1 and
        a.y2 == b.y2 and
        a.z1 == b.z1 and
        a.z2 == b.z2;
end

local function GetFlowGroups(info)
    local flowGroups = {}
    for k, mesh in pairs(info.meshes) do
        if mesh.vents then
            for kv, vent in pairs(mesh.vents) do
                local flow = FlowRate(info, vent)
                if flow ~= nil then
                    flowGroups[#flowGroups + 1] = {
                        device = vent,
                        flow = flow,
                        dimensions = vent.dimensions
                    }
                end
            end
        end
    end
    for k, device in pairs(info.devices) do
        if IsFlowDevice(device) then
            local matchingDevice = 0
            for i, group in ipairs(flowGroups) do
                if DimensionsMatch(group.dimensions, device.dimensions) then
                    matchingDevice = i
                    break
                end
            end
            if matchingDevice > 0 then
                flowGroups[matchingDevice].vol = device
            else
                flowGroups[#flowGroups + 1] = {
                    vol = device,
                    dimensions = device.dimensions,
                }
            end
        elseif IsSootFlowDevice(device) then
            local matchingDevice = 0
            for i, group in ipairs(flowGroups) do
                if DimensionsMatch(group.dimensions, device.dimensions) then
                    matchingDevice = i
                    break
                end
            end
            if matchingDevice > 0 then
                flowGroups[matchingDevice].soot = device
            else
                flowGroups[#flowGroups + 1] = {
                    soot = device,
                    dimensions = device.dimensions,
                }
            end
        end
    end
    return flowGroups
end

local function FlowGroupDeviceId(flowGroup)
    if flowGroup.device then return flowGroup.device.id end
end

local function FlowGroupFlowPrefix(flowGroup)
    local vol = flowGroup.vol
    local soot = flowGroup.soot
    if vol and soot then
        local showName = TrimPunctuationEnd(CommonPrefix(vol.id, soot.id))
        if showName:len() < 3 then
            showName = vol.id .. " " .. soot.id
        end
        return showName
    end
end

local function FlowGroupBestName(flowGroup)
    local deviceId = FlowGroupDeviceId(flowGroup)
    local prefix = FlowGroupFlowPrefix(flowGroup)
    if deviceId and deviceId:len() > 0 then
        return deviceId
    elseif prefix and prefix:len() > 0 then
        return prefix
    end
end

local function PlotFlowGroups(case, flowGroups, title, opts)
    if not opts then
        opts = {}
    end
    if not opts.avgWindow and opts.avgWindow ~= 0 then
        -- The default avergae window is 40 'frames'
        opts.avgWindow = 40
    end
    if not opts.dir then
        opts.dir = "."
    end
    local mechFlows = {}
    local vFlows = {}
    local sFlows = {}
    for i, flowGroup in ipairs(flowGroups) do
        local vol = flowGroup.vol
        local soot = flowGroup.soot
        local vs = {}
        local showName = FlowGroupBestName(flowGroup)
        local vFlow
        local sFlow
        if vol then
            vFlow = case.csvs['devc'].vectors[vol.id]
        end
        if soot then
            sFlow = case.csvs['devc'].vectors[soot.id]
        end
        -- TODO: we don't know whether to negate or not.
        if vFlow then
            vFlow = plot.wma(vFlow, opts.avgWindow)
            vFlow.y.name = "Volume Flow"
            vFlow.name = showName .. " Volume Flow"
            vs[#vs + 1] = vFlow
            vFlows[#vFlows + 1] = vFlow
            if flowGroup.device then
                mechFlows[#mechFlows + 1] = vFlow
            end
        end
        if sFlow then
            sFlow = plot.wma(sFlow, opts.avgWindow)
            sFlow.y.name = "Soot Mass Flow"
            sFlow.name = showName .. " Soot Mass Flow"
            vs[#vs + 1] = sFlow
            sFlows[#sFlows + 1] = sFlow
        end
        if #vs > 0 then
            plot.DV(opts.dir, vs,
                showName, opts.gnuplot)
        end
    end
    if #vFlows > 0 then
        plot.DV(opts.dir, vFlows,
            title .. " Volume Flows", opts.gnuplot)
    end
    if #sFlows > 0 then
        plot.DV(opts.dir, sFlows,
            title .. " Soot Mass Flows", opts.gnuplot)
    end
    if #mechFlows > 0 then
        plot.DV(opts.dir, mechFlows,
            title .. " Mechanical Volume Flows", opts.gnuplot)
    end
end

function affinity.autoFlowGroups(case, info)
    local flowGroups = GetFlowGroups(info)
    PlotFlowGroups(case, flowGroups, "Flows", { dir = "plots" })
    return flowGroups
end

function affinity.resiTimeline()
    return {
        aptDoor = {
            open = 300,
            close = 320
        },
        stairDoor = {
            open = 315,
            close = 335
        },
        corridorClearance = 440
    }
end

function GetSootRegions(case, info, pOpts)
    local sootRegions = {
        corridor = {},
        flat = {},
        stair = {},
        lobby = {},
    }

    local corridorSootNames = {}
    for i, v in ipairs(info.devices) do
        if v.quantities[1] == "DENSITY" and v.spec_id == "SOOT" and v.id:find("Corridor") then
            corridorSootNames[#corridorSootNames + 1] = v.id
            print("corridor", v.id)
        end
    end

    local corridorFunc = function(k, v)
        for i, name in ipairs(corridorSootNames) do
            if name == k then
                return true
            end
        end
        return false
    end

    local lobbySootNames = {}
    for i, v in ipairs(info.devices) do
        if v.quantities[1] == "DENSITY" and v.spec_id == "SOOT" and v.id:find("Lobby") then
            lobbySootNames[#lobbySootNames + 1] = v.id
        end
    end

    local lobbyFunc = function(k, v)
        for i, name in ipairs(lobbySootNames) do
            if name == k then
                return true
            end
        end
        return false
    end

    local flatSootNames = {}
    for i, v in ipairs(info.devices) do
        if v.quantities[1] == "DENSITY" and v.spec_id == "SOOT" and (v.id:find("Flat") or v.id:find("Apt") or v.id:find("Apartment")) then
            flatSootNames[#flatSootNames + 1] = v.id
        end
    end

    local flatFunc = function(k, v)
        for i, name in ipairs(flatSootNames) do
            if name == k then
                return true
            end
        end
        return false
    end

    local stairSootNames = {}
    for i, v in ipairs(info.devices) do
        if v.quantities[1] == "DENSITY" and v.spec_id == "SOOT" and v.id:find("Stair") then
            stairSootNames[#stairSootNames + 1] = v.id
        end
    end

    local stairFunc = function(k, v)
        for i, name in ipairs(stairSootNames) do
            if name == k then
                return true
            end
        end
        return false
    end

    sootRegions.corridor[#sootRegions.corridor + 1] = affinity.plotDevcs(case, corridorFunc,
        {
            transform = "sum",
            title = "Corridor Soot Total",
            name = "Corridor Soot Total",
            dir = pOpts.plotDir .. "/" .. "regions",
            yName = "Total Soot Quantity",
            gnuplot = { xrange = "[0:" .. pOpts.tend .. "]" }
        }
    )

    sootRegions.lobby[#sootRegions.lobby + 1] = affinity.plotDevcs(case, lobbyFunc,
        {
            transform = "sum",
            title = "Lobby Soot Total",
            name = "Lobby Soot Total",
            dir = pOpts.plotDir .. "/" .. "regions",
            yName = "Total Soot Quantity",
            gnuplot = { xrange = "[0:" .. pOpts.tend .. "]" }
        }
    )

    sootRegions.flat[#sootRegions.flat + 1] = affinity.plotDevcs(case, flatFunc,
        {
            transform = "sum",
            title = "Flat Soot Total",
            name = "Flat Soot Total",
            dir = pOpts.plotDir .. "/" .. "regions",
            yName = "Total Soot Quantity",
            gnuplot = { xrange = "[0:" .. pOpts.tend .. "]" }
        }
    )

    sootRegions.stair[#sootRegions.stair + 1] = affinity.plotDevcs(case, stairFunc,
        {
            transform = "sum",
            title = "Stair Soot Total",
            dir = pOpts.plotDir .. "/" .. "regions",
            yName = "Total Soot Quantity",
            name = "Stair Soot Total",
            gnuplot = { xrange = "[0:" .. pOpts.tend .. "]" },
            averaged = 300,
            retain = pOpts.plotDir .. "/stair_soot_quantity.json"
        }
    )
    return sootRegions
end

function affinity.autoSootRegions(case, info, pOpts)
    local sootRegions = GetSootRegions(case, info, pOpts)
    local regionSootLevels = {}
    if #sootRegions.corridor == 1 then
        regionSootLevels[#regionSootLevels + 1] = plot.wma(sootRegions.corridor[1], pOpts.avgWindow)
        sootRegions.corridor[1].name = "Corridor"
    elseif #sootRegions.corridor > 1 then
        error("multiple definitions of soot region")
    end
    if #sootRegions.flat == 1 then
        regionSootLevels[#regionSootLevels + 1] = plot.wma(sootRegions.flat[1], pOpts.avgWindow)
        sootRegions.flat[1].name = "Flat"
    elseif #sootRegions.flat > 1 then
        error("multiple definitions of soot region")
    end
    if #sootRegions.stair == 1 then
        regionSootLevels[#regionSootLevels + 1] = plot.wma(sootRegions.stair[1], pOpts.avgWindow)
        sootRegions.stair[1].name = "Stair"
    elseif #sootRegions.stair > 1 then
        error("multiple definitions of soot region")
    end
    if #sootRegions.lobby == 1 then
        regionSootLevels[#regionSootLevels + 1] = plot.wma(sootRegions.lobby[1], pOpts.avgWindow)
        sootRegions.lobby[1].name = "Lobby"
    elseif #sootRegions.lobby > 1 then
        error("multiple definitions of soot region")
    end

    if #regionSootLevels > 0 then
        plot.DV(pOpts.plotDir .. "/regions", regionSootLevels,
            "Soot Levels in Domain for " .. case.chid, { xrange = "[0:" .. pOpts.tend .. "]" })
    end
end

function affinity.keyRenders(smv, case, geom, timeline)
    -- Visibility
    local threshold = 30
    smv.view.colorbar.flip = true
    smv.load.slice_std(case, "SOOT VISIBILITY", 3, geom.occHeight)
    smv.bounds.slices.set("VIS_C", { min = 0, max = threshold * 2 })
    smv.camera.from_z_max()
    smv.clipping.mode = 2
    smv.clipping.z.max = geom.occHeight
    -- TODO: how do we get the time of extract activation.
    -- TODO: find and render the worst value for soot in stair
    if timeline then
        if timeline.aptDoor.close then
            smv.view.time = timeline.aptDoor.close
            print(smv.view.time)
            smv.render("Vis - 2 m AFFL - %.0fs (Flat Door Close)", smv.view.time)
            smv.view.time = timeline.stairDoor.close
            smv.render("Vis - 2 m AFFL - %.0fs (Stair Door Close)", smv.view.time)
            smv.bounds.slices.set("VIS_C", { min = 0, max = 20 })
            smv.view.time = timeline.corridorClearance
            smv.render("Vis - 2 m AFFL - %.0fs (Corridor Clearance)", smv.view.time)
            smv.bounds.slices.set("VIS_C", { min = 0, max = threshold * 2 })
        end
    end
    smv.view.set_time_end()
    smv.render("Vis - 2 m AFFL (End)", smv.view.time)
    smv.unload.all()

    if geom.stair then
        local threshold = 30
        smv.view.colorbar.flip = true
        smv.load.slice_std(case, "SOOT VISIBILITY", geom.stair.axis, geom.stair.offset)
        smv.bounds.slices.set("VIS_C", { min = 0, max = threshold * 2 })
        if geom.stair.axis == 1 then
            smv.camera.from_x_min()
        elseif geom.stair.axis == 2 then
            smv.camera.from_y_min()
        end
        smv.clipping.mode = 2
        smv.clipping.y.min = geom.stair.offset
        smv.camera.set_orthographic()
        if timeline then
            if timeline.aptDoor.close then
                smv.view.time = timeline.aptDoor.close
                smv.render("Vis - Stair View - %.0fs (Flat Door Close)", smv.view.time)
                smv.view.time = timeline.stairDoor.close
                smv.render("Vis - Stair View - %.0fs (Stair Door Close)", smv.view.time)
                smv.bounds.slices.set("VIS_C", { min = 0, max = 20 })
                smv.view.time = timeline.corridorClearance
                smv.render("Vis - Stair View - %.0fs (Corridor Clearance)", smv.view.time)
                smv.bounds.slices.set("VIS_C", { min = 0, max = threshold * 2 })
            end
        end
        smv.view.set_time_end()
        smv.render("Vis - Stair View (end)", smv.view.time)
        smv.clipping.y.min = nil
        smv.unload.all()
    end


    -- Speed
    smv.view.colorbar.flip = false
    smv.load.slice_std(case, "VELOCITY", 3, geom.occHeight)
    smv.bounds.slices.set("vel", { min = 0, max = 10 })
    smv.camera.from_z_max()
    smv.clipping.mode = 2
    smv.clipping.z.max = geom.occHeight
    if timeline then
        if timeline.aptDoor.close then
            smv.view.time = timeline.aptDoor.close
            smv.render("Air Speed - 2 m AFFL - %.0fs (Flat Door Close)", smv.view.time)
            smv.view.time = timeline.stairDoor.close
            smv.render("Air Speed - 2 m AFFL - %.0fs (Stair Door Close)", smv.view.time)
            smv.view.time = timeline.corridorClearance
            smv.render("Air Speed - 2 m AFFL - %.0fs (Corridor Clearance)", smv.view.time)
        end
    end
    smv.view.set_time_end()
    smv.render("Air Speed - 2 m AFFL (end)", smv.view.time)
    smv.unload.all()

    -- Temperature
    smv.view.colorbar.preset = "AFAC split"
    smv.view.colorbar.flip = false
    smv.bounds.slices.set("temp", { min = 20, max = 275 })
    smv.load.slice_std(case, "TEMPERATURE", 3, geom.ffHeight)
    smv.bounds.slices.set("temp", { min = 20, max = 275 })
    smv.camera.from_z_max()
    smv.clipping.mode = 2
    smv.clipping.z.max = geom.occHeight
    if timeline then
        if timeline.aptDoor.close then
            smv.view.time = timeline.aptDoor.close
            smv.render("Temp AFAC - 1.5 m AFFL - %.0fs (Flat Door Close)", smv.view.time)
            smv.view.time = timeline.corridorClearance
            smv.render("Temp AFAC - 1.5 m AFFL - %.0fs (Corridor Clearance Time)", smv.view.time)
        end
    end
    smv.view.set_time_end()
    smv.render("Temp AFAC - 1.5 m AFFL (end)", smv.view.time)
    smv.unload.all()
    -- Restore house style
    affinity.setHouseStyle(smv)

    smv.view.colorbar.flip = false
    smv.load.slice_std(case, "TEMPERATURE", 3, geom.ffHeight)
    smv.bounds.slices.set("temp", { min = 20, max = 300 })
    smv.camera.from_z_max()
    smv.clipping.mode = 2
    smv.clipping.z.max = geom.occHeight
    if timeline then
        if timeline.aptDoor.close then
            smv.view.time = timeline.aptDoor.close
            smv.render("Temp 160 - 1.5 m AFFL - %.0fs (Flat Door Close)", smv.view.time)
            smv.view.time = timeline.corridorClearance
            smv.render("Temp 160 - 1.5 m AFFL - %.0fs (Corridor Clearance Time)", smv.view.time)
        end
    end
    smv.view.set_time_end()
    smv.render("Temp 160 - 1.5 m AFFL (end)", smv.view.time)
    smv.unload.all()

    -- Pressure
    smv.load.slice_std(case, "PRESSURE", 3, geom.occHeight)
    smv.bounds.slices.set("pres", { min = -130, max = 10 })
    smv.camera.from_z_max()
    smv.clipping.mode = 2
    smv.clipping.z.max = geom.occHeight
    smvlib.setcolorbarflip(true)
    if timeline then
        if timeline.aptDoor.close then
            smv.view.time = timeline.aptDoor.close
            smv.view.colorbar.flip = true
            smv.render("Pres - 2 m AFFL - %.0fs (apt door re-close)", smv.view.time)
            smv.view.time = timeline.corridorClearance
            smv.view.colorbar.flip = true
            smv.render("Pres - 2 m AFFL - %.0fs (apt door re-close + 120)", smv.view.time)
        end
    end
    smv.view.colorbar.flip = true
    smv.view.set_time_end()
    smv.render("Pres - 2 m AFFL (end)", smv.view.time)
    smv.unload.all()
end

function affinity.autoPlotHRR(case, pOpts, opts)
    local hrrDV = case.csvs['hrr'].vectors['HRR']
    plot.plotHRRDV(pOpts.plotDir, hrrDV, string.format("HRR %s", case.chid),
        {},
        opts)
end

return affinity
