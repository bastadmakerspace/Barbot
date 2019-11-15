const express = require('express')
const app = express()
var path = require('path');
const WebSocket = require('ws');
const watch = require('node-watch')
const shell = require('child_process');

const port = 80

const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const arduino = new SerialPort('/dev/ttyACM0', { baudRate: 9600 });
const parser = arduino.pipe(new Readline({ delimiter: '\n' }));

const wss = new WebSocket.Server({ port: 8080 });

app.get('/', function(req, res) {
	res.sendFile(path.join(__dirname + '/index.html'));
});
app.get('/jquery.min.js', function(req, res) {
	res.sendFile(path.join(__dirname + '/jquery.min.js'));
});
app.get('/reconnecting-websocket.min.js', function(req, res) {
	res.sendFile(path.join(__dirname + '/reconnecting-websocket.min.js'));
});

watch(path.join(__dirname + '/index.html'), function(event, filename) {
  	console.log(filename, ' changed.');
   	wss.clients.forEach(function each(client) {
		if (client.readyState === WebSocket.OPEN) {
	    	client.send('reload');
		}
	});
})


app.post('/send', function(req, res) {
	var bodyStr = '';
    req.on("data",function(chunk){
        bodyStr += chunk.toString();
    });
    req.on("end",function(){
        // res.send(bodyStr);
        console.log(bodyStr);
		arduino.write(bodyStr, (err) => {
			if (err) {
				return console.log('Error on write: ', err.message);
			}
			console.log('message written');
			res.send('OK');
		});
    });
});

app.listen(port, () => console.log(`Listening on port ${port}!`))

// Websocket
wss.on('connection', function connection(ws) {
  ws.on('message', function incoming(data) {
  	// data = JSON.stringify(JSON.parse(data));
    console.log('ws received: %s', data);
    if (data == "off"){
    	shell.exec('sudo shutdown -h now');
    }
    else{
		arduino.write(data, (err) => {
			if (err) {
				return console.log('Error on write: ', err.message);
			}
			console.log('message written');
		});
	}
  });
});

// Read the port data
arduino.on("open", () => {
  console.log('serial port open');
});
parser.on('data', data =>{
  console.log('From arduino:', data);
 	wss.clients.forEach(function each(client) {
	  if (client.readyState === WebSocket.OPEN) {
	    client.send(data);
	  }
	});
});
