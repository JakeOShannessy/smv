import * as path from "jsr:@std/path";
import { JsonRpcClientUnix } from "./jsonrpcunix.ts";
import { JsonRpcClientWin } from "./jsonrpcwin.ts";
export type JsonRpcClient = JsonRpcClientUnix | JsonRpcClientWin;

export class Smokeview {
    private command: Deno.Command;
    private p: Deno.ChildProcess;
    private encoder: TextEncoder;
    public socketPath: string;
    constructor() {
        // const cmd =
        // "C:\\Users\\josha\\Documents\\smv\\dist-debug\\bin\\smokeview.exe";
        const cmd = Deno.build.os === "windows" ? "smvlua.cmd" : "smvlua";
        this.socketPath = Deno.makeTempDirSync({ suffix: ".smv.socket.dir" });
        this.socketPath = path.join(this.socketPath, "socket");
        this.command = new Deno.Command(cmd, {
            stdin: "null",
            stdout: "inherit",
            stderr: "inherit",
            args: [
                "-socket",
                this.socketPath,
                "C:\\Users\\josha\\Documents\\AFF_20498_05_G10_F3_V1_R2\\AFF_20498_05_G10_F3_V1_R2.smv",
            ],
        });
        this.p = this.command.spawn();
        this.p.ref();
        this.encoder = new TextEncoder();
    }
    async launch(): Promise<SmvRpc> {
        try {
            // poll until socket file exists. We can't use stat to be compatible
            // with windows.
            console.log(this.socketPath);
            let exists = false;
            const socketDir = path.dirname(this.socketPath);
            while (!exists) {
                for await (const dirEntry of Deno.readDir(socketDir)) {
                    console.log(dirEntry.name);
                    if (dirEntry.name === path.basename(this.socketPath)) {
                        exists = true;
                        break;
                    }
                }
            }
            return new SmvRpc(this);
        } catch (e) {
            this.close();
            throw e;
        }
    }
    close() {
        // console.error("killing");
        // this.p.kill("SIGINT");
        // Deno.kill(this.p.pid, "SIGINT");
        this.p.ref();
    }
    [Symbol.dispose]() {
        this.close();
    }
}

export class SmvRpc {
    public rpc: JsonRpcClient;
    constructor(private process: Smokeview) {
        this.rpc = new JsonRpcClientWin(process.socketPath);
    }
    async setAffinityStyle() {
        // -- Set the color bar to a blue/red split.
        await this.rpc.call("set_colorbar", ["blue->red split"]);
        // -- Set the window size to 800×500
        await this.rpc.call("set_window_size", { width: 800, height: 500 });
        // -- Don't show the title of the model
        await this.rpc.call("set_title_visibility", [false]);
        // -- Don't show the version of smokeview being used.
        await this.rpc.call("set_smv_version_visibility", [false]);
        // -- Show the CHID of the model
        await this.rpc.call("set_chid_visibility", [true]);
        // -- Don't show blockages.
        await this.rpc.call("blockages_hide_all");
        // -- Don't show any surfaces
        await this.rpc.call("surfaces_hide_all");
        // -- Hide domain outlines
        await this.rpc.call("outlines_hide_all");
        // -- Don't show any devices
        await this.rpc.call("devices_hide_all");
        // -- Use a large font
        await this.rpc.call("set_font_size", ["large"]);

        await this.rpc.call("set_render_dir", ["renders"]);
        await this.rpc.call("set_render_type", ["rendePNGrs"]);
    }
    async call(method: string, params?: any): Promise<any> {
        return this.rpc.call(method, params);
    }
    async notify(method: string, params?: any): Promise<any> {
        return this.rpc.notify(method, params);
    }
    close() {
        this.rpc.close();
        this.process.close();
    }
}

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

export async function startSmvRpc(): Promise<SmvRpc> {
    const smv = new Smokeview();
    const smvRpc = await smv.launch();
    return smvRpc;
}
const smvRpc = await startSmvRpc();
await smvRpc.setAffinityStyle();
await smvRpc.call("set_clipping", { mode: 2, x: { max: 2 } });
await smvRpc.call("set_chid_visibility", [true]);
await smvRpc.call("set_window_size", { width: 800, height: 500 });
const smoke3ds = await smvRpc.call("get_smoke3ds");
const smokeIndices = smoke3ds.filter((
    c: { longlabel: string; index: number },
) => c.longlabel === "SOOT DENSITY");
await smvRpc.call(
    "load_smoke3d_indices",
    smokeIndices.map((c: { longlabel: string; index: number }) => c.index - 1),
);
await smvRpc.call("set_frame", [500]);
await smvRpc.call("render");
await smvRpc.call("unload_all");
const slices = await smvRpc.call("get_slices");
const meshes = await smvRpc.call("get_meshes");

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
await smvRpc.call(
    "load_slice_indices",
    sliceIndices.map((c: { longlabel: string; index: number }) => c.index - 1),
);
await smvRpc.call("set_ortho_preset", ["XMAX"]);
await smvRpc.call("set_clipping", { mode: 2, x: { max: 2.45 } });
await smvRpc.call("set_time", [255]);
await smvRpc.call("set_projection_type", [1]);
await smvRpc.call("set_colorbar", ["blue->red split"]);
await smvRpc.call("set_font_size", ["large"]);
await smvRpc.call("set_slice_bounds", {
    type: "temp",
    set_min: true,
    value_min: 20,
    set_max: true,
    value_max: 160,
});
await smvRpc.call("render", { basename: "temperature" });
await smvRpc.notify("exit");
smvRpc.close();
