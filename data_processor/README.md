# Data Processor
This demo program aggregates a certain number of data and calculates average using WebAssembly. The result is sent via WiFi mesh. 

### WebAssembly module
There is an example AssemblyScript code under [assemblyscript](https://github.com/internet-of-production/WasmMeshDemo/tree/main/assemblyscript). 
The Wasm binary is under 'data' directory (due to default upload setting of platform io). This example Wasm is compiled with asc cli options '-O3 --noAssert --use abort='
