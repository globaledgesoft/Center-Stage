var async = require('async');
var config = require('./config.json');

var Setup = function(app){
    this.app = app;
    this.knex = app.knex;
    this.config = app.config;
};
module.exports = Setup;
//console.log(this.app);
var createSchema = function(callback){
    knex.raw('create schema if not exists '+config.APP_DB_CONFIG.dbConfig.schemaName)
    .asCallback(function(err, result){
        if(!err){
            callback(null, 'Schema creation done');
        } else {
            callback(null, 'Schema already exists');
        }
    });
};

var createTable = function(callback){
    knex.schema.withSchema(config.APP_DB_CONFIG.dbConfig.schemaName).hasTable('music').then(function(exists) {
        if (!exists) {
            return knex.schema.createTable('music', function(table) {
                table.increments('id').primary();
                table.string('musicName',50);
                table.integer('download').notNullable;
            });
        }
    }).asCallback(function(err,rows){
        if(!err){
            callback(null, "music table created");
        }
        else{
            console.log(err)
            callback(null, "music table creation error");
        }
    })
}

var createCrowdTable = function(callback){
    knex.schema.withSchema(config.APP_DB_CONFIG.dbConfig.schemaName).hasTable('crowd').then(function(exists) {
        if (!exists) {
            return knex.schema.createTable('crowd', function(table) {
                table.increments('id').primary();
                table.integer('hour').notNullable;
                table.integer('minute').notNullable;
                table.integer('value').notNullable;
            });
        }
    }).asCallback(function(err,rows){
        if(!err){
            callback(null, "crowd table created");
        }
        else{
            console.log(err)
            callback(null, "crowd table creation error");
        }
    })
}

var deleteMusicData = function(callback){
    knex.raw('drop table music')
    .asCallback(function(err, result){
        if(!err){
            callback(null, 'table deletion done');
        } else {
            callback(null, 'table deletion failed');
        }
    });
};

var deleteCrowdData = function(callback){
    knex.raw('drop table crowd')
    .asCallback(function(err, result){
       
        if(!err){
            callback(null, 'table deletion done for crowd');
        } else {
            callback(null, 'table deletion failed for crowd');
        }
    });
};

var insertMusicData = function(callback){
    knex.withSchema(config.APP_DB_CONFIG.dbConfig.schemaName).select().table('music')
    .asCallback(function(err, rows){
        if(!err && rows[0]){
            callback(null, "Insertion into Music data failed");
        } else {
            knex.withSchema(config.schemaName).table('music')
            .insert([{
                musicName : "Rock",
                download : 25
            },{
                musicName : "EDM",
                download : 25
            },{
                musicName : "Hip Hop",
                download : 25
            },{
                musicName : "Pop",
                download : 25
            }]).asCallback(function(err, rows){
                if(!err){
                    callback(null, "Music data inserted");
                } else {
                    console.log(err)
                    callback(null, "Music data insertion failed");
                }
            });
        }
    });
};

var insertCrowdData = function(callback){
    knex.withSchema(config.APP_DB_CONFIG.dbConfig.schemaName).select().table('crowd')
    .asCallback(function(err, rows){
        if(!err && rows[0]){
            callback(null, "Insertion into Crowd data failed");
        } else {
            knex.withSchema(config.APP_DB_CONFIG.dbConfig.schemaName).table('crowd')
            .insert([{
                hour:7,
                minute:30,
                value:34
            },{
                hour:8,
                minute:00,
                value:45 
            },{
                hour:8,
                minute:30,
                value:27
            },{
                hour:9,
                minute:00,
                value:50
            },{
                hour:9,
                minute:30,
                value:67
            },{
                hour:10,
                minute:00,
                value:54
            },{
                hour:10,
                minute:30,
                value:74
            },{
                hour:11,
                minute:00,
                value:34
            },{
                hour:11,
                minute:30,
                value:84
            }]).asCallback(function(err, rows){
                if(!err){
                    callback(null, "Crowd data inserted");
                } else {
                    console.log(err)
                    callback(null, "Crowd data insertion failed");
                }
            });
        }
    });
};




Setup.prototype.setup = function(){
    async.series({
        createSchema : createSchema,
        createTable : createTable
    }, function(err, results){
        console.log(results);
    });
};  
