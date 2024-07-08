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

const client = new JsonRpcClient();
// const r = await client.call("subtract", [42, 23]);
// console.log("r:", await client.call("subtract", [42, 23]));
console.log("r:", await client.call("subtract", [47, 1]));
console.log("r:", await client.call("set_frame", [500]));
console.log(
    "r:",
    await client.call("set_clipping", { mode: 2, x: { max: 2 } }),
);
await client.call("set_chid_visibility", [true]);
await client.call("set_window_size", { width: 1600, height: 1000 });
console.log(
    "r:",
    await client.call("render", {}),
);
console.log("r:", await client.call("unload_all"));
// TODO: exit should be a notification
console.log("r:", await client.call("exit"));

conn.close();
