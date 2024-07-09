import { SplitJsonObjectsStream } from "./rstream.ts";

const conn = await Deno.connect({
    transport: "unix",
    path: "/home/jake/couch/echo_socket",
});

class JsonRpcClient {
    private writeStream = new TextEncoderStream();
    private writer;
    private responseStream: ReadableStream<object>;
    private textStream: ReadableStream<string>;
    private objectStream: ReadableStream<object>;
    private reader: ReadableStreamDefaultReader<object>;
    private n = 0;
    constructor() {
        /*await*/ this.writeStream.readable.pipeTo(conn.writable);
        this.writer = this.writeStream.writable.getWriter();
        this.textStream = conn.readable.pipeThrough(
            new TextDecoderStream(),
        );
        this.objectStream = this.textStream.pipeThrough(
            new SplitJsonObjectsStream(),
        );
        this.reader = this.objectStream.getReader();
        this.responseStream = new ReadableStream({
            start(controller) {
                /* … */
            },

            async pull(controller) {
                // let response = "";
                // const objectStrings = [];
                // let singleObj = "";
                // const textStream = conn.readable.pipeThrough(
                //     new TextDecoderStream(),
                // );
                // const reader = textStream.getReader();
                // // while (true) {
                // const { value, done } = await reader.read();
                // if (value) {
                //     // const s = decoder.decode(value);
                //     console.log("s:", value);
                //     for (const c of value) {
                //         singleObj += c;
                //         if (c == "}") {
                //             objectStrings.push(singleObj);
                //             singleObj = "";
                //             // break;
                //         }
                //         // console.log(c);
                //     }
                //     console.log(objectStrings);
                //     response += value;
                // }
                // //     if (done) {
                // //         break;
                // //     }
                // // }
                // // console.log(response);
                // const t: object[] = objectStrings.map((s) => JSON.parse(s));
            },

            cancel(reason) {
                /* … */
            },
        });
    }
    async send(obj: object) {
        await this.writer.write(JSON.stringify(obj));
    }
    // TODO: create a stream of JSON objects
    async recv(): Promise<object | undefined> {
        return (await this.reader.read()).value;
    }
    async call(method: string, params?: any) {
        await client.send({
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
            "id": this.n,
        });
        this.n++;
        // TODO: make sure ids correspond
        const r = (await this.reader.read()).value;
        if (isJsonRpcResponse(r)) {
            return r.result;
        } else {
            console.error(r);
        }
    }
    async notify(method: string, params?: any) {
        await client.send({
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
        });
    }
}
export interface JsonRpcResponse {
    jsonrpc: "2.0";
    id: any;
    result?: any;
    error?: any;
}
function isJsonRpcResponse(o: any): o is JsonRpcResponse {
    return o["jsonrpc"] === "2.0" && "id" in o && "result" in o;
}

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

const client = new JsonRpcClient();
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

const distance = 2.45;
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
// await client.call("unload_all");
// await client.notify("exit");
conn.close();

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
