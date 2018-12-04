//Initializing the required packages
var express = require('express');
var pg = require("pg");
var app = express();
var path = require('path');
var apiRoutes = express.Router();
var bodyParser = require('body-parser');
var async = require("async");
var config = require("./config/config.json");
const { spawn } = require('child_process');
const startBulbScript = path.join(__dirname, 'serial_write.py');

//Initializing socket communication
var http = require('http').Server(app);
io = require('socket.io')(http);

const startMotionDetection = spawn('python', [startBulbScript,"4 4 1 " + config.SERIAL_COMM.duration_in_sec + " " + config.SERIAL_COMM.frequency_threshold]);
startMotionDetection.stdout.on('data', (data) => {
	console.log(`stdout(start motion detection) : ${data}`);
});
startMotionDetection.stderr.on('data', (data) => {
	console.log(`stderr(start motion detection ) : ${data}`);
});
startMotionDetection.on('close', (code) => {
	console.log(`stopped(start motion detection) : ${code}`);
});

app.set('view engine', 'html');
app.set('views', __dirname + '/webapps');
app.set('view engine', 'html');

//Configuring static pages directory.
app.use(express.static(path.join(__dirname + '/webapps')));
app.use(express.static(path.join(__dirname, 'public')));

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended : true }));  


app.use(function(req,res,next){
	res.header("Access-Control-Allow-Origin","*");
	res.header("Access-Control-Allow-Headers","Origin,X-Requested-With,Content-Type,Accept,Authorization");
	res.header("Access-Control-Allow-Methods","GET,POST");
	next();
});
//Creating knex query manager Object
knex    	 = require('knex')(config.APP_DB_CONFIG.dbConfig);

app.knex = knex;
app.config = config;

//Configuring and creating tables.
var Setup = require('./config/setup.js');
var setupObject = new Setup(app);

//Configuring APIRoutes
var Routes = require('./controller/routes.js')
var routes = new Routes(app);
routes.init();

module.exports = app;

setupObject.setup(function(err, res) {
    if(!err && (res ==  "done")) {
        console.log("done");
    }
});

//Configuring and starting the API server.
var server = http.listen(config.APP_DB_CONFIG.port, function () {
  	var host = server.address().address;
  	var port = server.address().port;
  	console.log("Example app listening at http://%s:%s", host, port);
});
