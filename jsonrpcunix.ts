import { isJsonRpcResponse } from "./jsonrpccommon.ts";
import { SplitJsonObjectsStream } from "./rstream.ts";

const conn = await Deno.connect({
    transport: "unix",
    path: "./echo_socket",
});

export class JsonRpcClientUnix {
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
        await this.send({
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
        await this.send({
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
        });
    }
    close() {

    }
}
