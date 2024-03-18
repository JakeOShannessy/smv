--- @module 'plot'
local affinity = {}

local plot = require("plot")
local json = require("json")

function affinity.meshInfoTable(case)
    local s = ""
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
    s = s ..string.format("----------------------------------------------------------------") .. "\n"
    s = s ..string.format("#\t|\t     MeshId.\t|\t# Cells\t| I-J-K") .. "\n"
    s = s ..string.format("----------------------------------------------------------------") .. "\n"
    for k, mesh in ipairs(case.meshes) do
        local nCells = mesh.ibar * mesh.jbar * mesh.kbar
        totalCells = totalCells + nCells
        local s = string.format("%%d\t| %%%ds\t|\t%%d\t| %%%ds", maxNameLength, maxIJKLength)
        s = s ..string.format(s, k, mesh.label, nCells,
            string.format("%d-%d-%d", mesh.ibar, mesh.jbar, mesh.kbar)) .. "\n"
    end
    s = s ..string.format("----------------------------------------------------------------") .. "\n"
    s = s ..string.format("Total\t|\t        \t|\t%d\t|", totalCells) .. "\n"
    s = s ..string.format("----------------------------------------------------------------") .. "\n"
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
end

---@alias plotOpts { title: string, name: string, dir: string, yName: string, yUnits: string, gnuplot: table, averaged: number, retain: string, extraVecs: table, transform: string, averagedOnly: boolean}

--- Plot the sum of a set of devc vectors defined by a given pattern.
--- @param case table The case to plot from
--- @param pattern string Any vector names which match this pattern will be
---  plotted.
--- @param opts plotOpts Various options to control the output.
function affinity.plotDevcs(case, pattern, opts)
    local vecs = {};
    if not opts then
        opts = {}
    end
    for k, v in pairs(case.csvs['devc'].vectors) do
        if k:match(pattern) then
            vecs[#vecs + 1] = v
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

return affinity
