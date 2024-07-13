import { JsonRpcClientUnix } from "./jsonrpcunix.ts";
import { JsonRpcClientWin } from "./jsonrpcwin.ts";

export type JsonRpcClient = JsonRpcClientUnix | JsonRpcClientWin;

const client = new JsonRpcClientWin();
// const r = await client.call("subtract", [42, 23]);
// console.log("r:", await client.call("subtract", [42, 23]));
// console.log("r:", await client.call("subtract", [47, 1]));
// console.log("r:", await client.call("move_x", [0.5]));
// console.log("r:", await client.call("rotate", [0.5]));

async function setAffinityStyle() {
    // -- Set the color bar to a blue/red split.
    await client.call("set_colorbar", ["blue->red split"]);
    // -- Set the window size to 800×500
    await client.call("set_window_size", { width: 800, height: 500 });
    // -- Don't show the title of the model
    await client.call("set_title_visibility", [false]);
    // -- Don't show the version of smokeview being used.
    await client.call("set_smv_version_visibility", [false]);
    // -- Show the CHID of the model
    await client.call("set_chid_visibility", [true]);
    // -- Don't show blockages.
    await client.call("blockages_hide_all");
    // -- Don't show any surfaces
    await client.call("surfaces_hide_all");
    // -- Hide domain outlines
    await client.call("outlines_hide_all");
    // -- Don't show any devices
    await client.call("devices_hide_all");
    // -- Use a large font
    await client.call("set_font_size", ["large"]);

    await client.call("set_render_dir", ["renders"]);
    await client.call("set_render_type", ["rendePNGrs"]);
}

await setAffinityStyle();
await client.call("set_clipping", { mode: 2, x: { max: 2 } });
await client.call("set_chid_visibility", [true]);
await client.call("set_window_size", { width: 800, height: 500 });
const smoke3ds = await client.call("get_smoke3ds");
const smokeIndices = smoke3ds.filter((
    c: { longlabel: string; index: number },
) => c.longlabel === "SOOT DENSITY");
await client.call(
    "load_smoke3d_indices",
    smokeIndices.map((c: { longlabel: string; index: number }) => c.index - 1),
);
await client.call("set_frame", [500]);
await client.call("render");
await client.call("unload_all");
const slices = await client.call("get_slices");
const meshes = await client.call("get_meshes");

const distance = -10.4;
const sliceIndices = slices.filter((
    c: {
        shortlabel: string;
        index: number;
        idir: number;
        position_orig: number;
        mesh: number;
    },
) => {
    const mesh = meshes[c.mesh];
    const cellWidth = findCellDimension(mesh, c.idir, distance);
    if (!cellWidth) return false;
    return c.shortlabel === "temp" && c.idir === 1 &&
        c.position_orig > (distance - cellWidth * 0.25) &&
        c.position_orig < (distance + cellWidth * 0.25);
});
await client.call(
    "load_slice_indices",
    sliceIndices.map((c: { longlabel: string; index: number }) => c.index - 1),
);
await client.call("set_ortho_preset", ["XMAX"]);
await client.call("set_clipping", { mode: 2, x: { max: 2.45 } });
await client.call("set_time", [255]);
await client.call("set_projection_type", [1]);
await client.call("render", { basename: "temperature" });
await client.call("set_colorbar", ["blue->red split"]);
await client.call("set_font_size", ["large"]);
await client.call("set_slice_bounds", {
    type: "temp",
    set_min: true,
    value_min: 20,
    set_max: true,
    value_max: 160,
});
function sleep(seconds: number) {
    return new Promise((resolve) => setTimeout(resolve, seconds * 1000));
}
client.close();

function findCellDimension(
    mesh: {
        xplt_orig: number[];
        yplt_orig: number[];
        zplt_orig: number[];
        i: number;
        j: number;
        k: number;
    },
    axis: number,
    distance: number,
): number | undefined {
    let orig_plt;
    let bar;
    if (axis === 1) {
        orig_plt = mesh.xplt_orig;
        bar = mesh.i;
    } else if (axis === 2) {
        orig_plt = mesh.yplt_orig;
        bar = mesh.j;
    } else if (axis === 3) {
        orig_plt = mesh.zplt_orig;
        bar = mesh.k;
    } else {
        throw new Error("invalid axis");
    }
    //   -- TODO: Account for being slightly out.
    for (let i = 0; i < bar - 2; i++) {
        if (orig_plt[i] <= distance && distance <= orig_plt[i + 1]) {
            return (orig_plt[i + 1] - orig_plt[i]);
        }
    }
    // TODO: currently this is just a fallback
    return 0.1;
}
