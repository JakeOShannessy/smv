# Smokeview Scripting

The core Lua code has been synced and substantial improvements have been made to
the API. A focus has beeen retaining more data in the host (i.e. the 'C' part of
smokeview ) and less in the script.

The sicking point in the API will be how how multiple cases are handled. If we
have multiple cased, do we have multiple active cameras? THis affects not just
the scripting but all of smokeview.

One option is to have 4 types of object:

1. `smv`, the program itself. There is only one.
2. `view`, essentially the rectangular window render area. It can render multiple caes.
3. `caseview` contains the camera and data visibility information for a case. It
   'links' a case to a view. A single view might have multiple caseviews that
   reference a single case (to show different/angles data).
4. `case` the cae and data itself.

Calculations on data and teh loading of data is done via 'case'.

Selection and showing of data is done via `caseview`. This includes things such
as camera and clipping.

The arrangement and combination of `caseview`s is done via `view` all rendering operations are applied to `view` it's perfectly fine to ahve multiple views.

`smv` provides program-wide information and functions, as well as the starting
point for the other 3 types.

```lua
local view = smv.create_view()
local case = smv.load_case("path/file./smv")
local caseview = case.create_view()
caseview.set_camera(x)
view.link(caseview)
```

This is flexible but some sensible defaults are in order.

```lua
local view, vis, case = smv.load_default_case()
```

This line is present at the start of every script when using the current
smokeview version. It should be future compatible with new capabilities.

Generic libraries like plotting, file access, etc are imported as per regular lua models as they operate separately from smokeview. Thjis includes some modules which are bundled with smokeview. Example:

```lua
case.load(_)
vis.show(_)
vis.time = _
view.render()
```
