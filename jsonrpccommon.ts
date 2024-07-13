export interface JsonRpcResponse {
    jsonrpc: "2.0";
    id: any;
    result?: any;
    error?: any;
}
export function isJsonRpcResponse(o: any): o is JsonRpcResponse {
    return o["jsonrpc"] === "2.0" && "id" in o && "result" in o;
}
