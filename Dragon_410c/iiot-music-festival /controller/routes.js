const { spawn } = require('child_process');
const path = require('path');
const bluetoothScript = path.join(__dirname, '../../bluetooth_playback/play_song.py');
const startBulbScript = path.join(__dirname, '../serial_write.py');
const request = require('request');
var config = require("../config/config.json")

var Routes = function(app) {
    this.app = app;
    //this.indexImplInstance = new IndexImpl(app);
    this.config = app.config;
    
};
module.exports = Routes;

Routes.prototype.init = function() {

    var routesApp = this.app;
    var self = this;
	
   routesApp.get('/getMovements',function(req,res){
	   request.get('http://54.153.108.137:8654/getMoments',function(err, response, movementsData){
            //console.log("movementsData");
	        //console.log(movementsData);
			res.send(movementsData);
   	   })
   });
   routesApp.get('/getMusicData',function(req,res){
        var responseObject = {
            status: true,
            code: 200
        };
        knex.withSchema(config.withSchema).table('music').select().asCallback(function(err,rows){
           if(err){
            responseObject.message = "failed to retreive data";
            responseObject.data = null;
            res.json(responseObject);
           }
           if(!err){
            responseObject.message = "data retreived successfully";
            responseObject.data = rows
            res.json(responseObject);
           }
       })
   })

   routesApp.post('/singMusic',function(req,res){
        responseData = {
            status:'success',
        }

        const startBluetooth = spawn('python', [bluetoothScript,req.body.music_type]);
        startBluetooth.stdout.on('data', (data) => {
            console.log(`stdout: ${data}`);
           // res.send(data);
        });
        startBluetooth.stderr.on('data', (data) => {
            console.log(`stderr: ${data}`);
        });
        startBluetooth.on('close', (code) => {
            console.log('closed');
            console.log(`child process exited with code ${code}`);
            
            const stopMotionDetection = spawn('python', [startBulbScript, config.SERIAL_COMM.stop_music]);
            stopMotionDetection.stderr.on('data', (data) => {
                console.log(`stderr (stop motion detection ) : ${data}`);
            });
            stopMotionDetection.on('close', (code) => {
                console.log(`stopped(stop motion detection) : ${code}`);
            });
            res.send(responseData);
        });

        const startMotionDetection = spawn('python', [startBulbScript,config.SERIAL_COMM.start_music]);
        startMotionDetection.stdout.on('data', (data) => {
            console.log(`stdout(start motion detection) : ${data}`);
        });
        startMotionDetection.stderr.on('data', (data) => {
            console.log(`stderr(start motion detection ) : ${data}`);
        });
        startMotionDetection.on('close', (code) => {
            console.log(`stopped(start motion detection) : ${code}`);
        });
   })  

   routesApp.get('/getCrowdDetails',function(req,res){
        var responseObject = {
            status: true,
            code: 200
        };
        knex.withSchema(config.withSchema).table('crowd').select().asCallback(function(err,rows){
        if(err){
            responseObject.message = "failed to retreive data";
            responseObject.data = null;
            res.json(responseObject);
        }
        if(!err){
            responseObject.message = "data retreived successfully";
            responseObject.data = rows
            res.json(responseObject);
        }
    })
   })

   routesApp.post('/addMusic',function(req,res){
        var responseObject = {
            status: true,
            code: 200
        };
        knex.withSchema(config.withSchema).table('music').where('id','=',req.body.music_id).asCallback(function(err,rows){
            if(rows.length == 0){
                knex.withSchema(config.withSchema).table('music').insert({id:req.body.music_id,musicName:req.body.music_name,download:1}).asCallback(function(err,rows){
                    if(rows){
                        responseObject.message = "data inserted successfully";
                        responseObject.data = rows
                        res.json(responseObject); 
                    }
                    else{
                        responseObject.message = "failed to retreive data";
                        responseObject.data = null;
                        responseObject.code = 202;
                        res.json(responseObject);        
                    }
                })
            } 
            else{
                knex.withSchema(config.withSchema).table('music').where('id', '=', req.body.music_id).increment('download', 1).asCallback(function(err,rows){
                    if(rows){
                        responseObject.message = "data inserted successfully";
                        responseObject.data = rows
                        res.json(responseObject); 
                    }
                    else{
                        responseObject.message = "failed to insert data";
                        responseObject.data = null;
                        responseObject.code = 202;
                        res.json(responseObject);        
                    }
                })
            }
        });

   })

};
