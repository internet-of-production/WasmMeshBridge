const {SerialPort} = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

//Set your serial port path
const port = new SerialPort({
    //path:'/dev/cu.usbserial-142120',
    path:'/dev/cu.usbserial-0001',
    baudRate: 115200,
    autoOpen: false
})

const serial_read = function() {

    port.open(function (err){
        if (err) {
            return console.log('Error opening port: ', err.message)
        }
    })

    /*
    * Buffer must be flushed once, because some bytes are already written after opening ports.
    * Auto-open is disabled to set the flush.
    */
    port.on('open', function (){
        port.flush(function (err){
            if(err){
                console.log(err)
            }
        })
    })

    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' })) //Use Serial.println in arduino code. Otherwise, messages can not be delimited.
    parser.on('data', function(data){
        console.log('Data: '+ data);
        mqtt_publish(data)
    });
      
}

//MQTT
const mqtt = require('mqtt')
const topic = 'WasmMesh'
const subTopic = "WasmMesh"
//APPLY YOUR SETTING 
const options = {
    clientId: "MeshBridge",
    port:1883,
    host:"localhost",
    username:"wasmretrofitting",
    password:"wasmretrofitting",
    //key:KEY,
    //cert: CERT,
    //ca: TRUSTED_CA_LIST,
    rejectUnauthorized: false,
    protocol: "mqtt",
    reconnectPeriod:1000
}
const client  = mqtt.connect(options);
// https://www.emqx.io/mqtt/public-mqtt5-broker
//const client = mqtt.connect('mqtt://broker.emqx.io');

//Subscribe the broker
client.on('connect', function () {
    client.subscribe(subTopic, function (err) {
        if (!err) {
            console.log('Connected')
        }
    })
})

client.on('message', function (subTopic, message) {
    // message is Buffer
    console.log("Subscribe: " + message)
    //client.end()
})

//Display Errors
client.on('error', function(err) {
    console.dir(err)
})

const mqtt_publish = function (msg){
    //Check the connection
    if(!client.connected){
        client.reconnect()
    }
    client.publish(topic, msg.toString())
}

serial_read()