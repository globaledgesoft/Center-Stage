var mainApp = angular.module('mainPage',[
    'ngRoute',
]);

mainApp.controller('mainCtrl',function($scope){

})


mainApp.config(function($routeProvider,$locationProvider) {
    $routeProvider
    .when("/", {
        templateUrl : "./html/welcome.html",
        controller : "welcomeCtrl"
    })
    .when("/spinTunes", {
        templateUrl : "./html/spinTunes.html",
        controller : "spinTunesCtrl"
    })
    .when("/music", {
        templateUrl : "./html/chooseMusic.html",
        controller : "chooseMusicCtrl"
    })
    .when('/lightSensors',{
        templateUrl : "./html/lightSensors.html",
        controller : "sensorsCtrl"
    })
    .when('/assistance',{
        templateUrl : "./html/assistance.html",
        controller : "assistanceCtrl"
    })
    .when('/twitterReview',{
        templateUrl : "./html/twitterReview.html",
        controller : "twitterReviewCtrl"
    })
    .when('/optimize',{
        templateUrl : "./html/optimize.html",
        controller : "optimizeCtrl"
    })
    .when('/dashboard',{
        templateUrl : "./html/dashboard.html",
        controller : "dashboardCtrl"
    });
    /* $locationProvider.html5Mode(true); */
});

mainApp.controller('welcomeCtrl',function($scope,$location){
    $scope.goToSecondPage = function(){
        $location.path('/spinTunes')
    }
    $scope.goToDashboard = function(){
        $location.path('/dashboard')
    }
});

mainApp.controller('spinTunesCtrl',function($scope,$location){
    $scope.gotoMusicSelection = function(){
        $location.path('/music')
    }
    $scope.gotoDJ = function(){
        $location.path("/twitterReview")
    }
});

mainApp.controller('chooseMusicCtrl',function($scope,$location,getDetails){
    $scope.gotoLightSensors = function(music_type,music_id){
        getDetails.addMusic(music_id,music_type).then(function onSuccess(data){
            if(data.code == '200'){
                console.log("added to database");
            }
            else{
                console.log("error in adding");
            }
        });

        getDetails.singMusic(music_type).then(function onSuccess(data){
            if(data){
		console.log("in go to assistnce");
                $location.path('/assistance')
            }
        })
        $location.path("/lightSensors")
    }
});

mainApp.controller('sensorsCtrl',function($scope,$location,$interval){
    /* var socket = io.connect('http://localhost:8000', function(data) {
        console.log("connection created");
    });
    socket.on('stopMusicEventEmit', function(data){
        console.log('data',data.description)
        $location.path('/Assistance')
    }); */


    x=0;
    $scope.twitter_messages_4 = [{heading : "rustysteel-1m",message:"what a great music festival.Having an awesome time.#QF15Rules",color:'rgb(210, 81, 70)'},
                                {heading : "lukewarm-1m",message:"SOOOO much fun.#QualcommFest2018",color:"rgb(150, 106, 175)"},
                                {heading : "chrisbacon-2m",message:"QualcommFest2018 rocks!",color:"rgb(80, 115, 240)"},
                                {heading : "rustysteel-1m",message:"what a great music festival.Having an awesome time.#QF15Rules",color:"black"}];

    var twitter_messages = [
        {heading : "lukewarm-1m",message:"SOOOO much fun.#QualcommFest2018",color:"rgb(210, 81, 70)"},
        {heading : "chrisbacon-2m",message:"QualcommFest2018 rocks!",color:"rgb(150, 106, 175)"},
        {heading : "rustysteel-1m",message:"what a great music festival.Having an awesome time.#QF15Rules",color:"rgb(80, 115, 240)"},
        {heading : "lukewarm-1m",message:"SOOOO much fun.#QualcommFest2018",color:"black"},
        {heading : "chrisbacon-2m",message:"QualcommFest2018 rocks!",color:"rgb(210, 81, 70)"},
        {heading : "rustysteel-1m",message:"what a great music festival.Having an awesome time.#QF15Rules",color:"rgb(150, 106, 175)"},
        {heading : "lukewarm-1m",message:"SOOOO much fun.#QualcommFest2018",color:"rgb(80, 115, 240)"},
        {heading : "chrisbacon-2m",message:"QualcommFest2018 rocks!",color:"black"},
        {heading : "rustysteel-1m",message:"what a great music festival.Having an awesome time.#QF15Rules",color:"rgb(210, 81, 70)"},
        {heading : "lukewarm-1m",message:"SOOOO much fun.#QualcommFest2018",color:"rgb(150, 106, 175)"},
   ]

    $interval(function(){
        $scope.twitter_messages_4=[];
        for(var j = 0;j<4;j++){
            if(x > ( twitter_messages.length - 1)){
                x = 0;
            }
            $scope.twitter_messages_4.push(twitter_messages[x]);
            x++;
        }
        if(x >= 3){
            x = x - 3;  
        }
    },5000)
});
mainApp.controller('assistanceCtrl',function($scope,$location){
    $scope.goToDashboard = function(){
        $location.path('/dashboard')
    }

    setTimeout(function(){ $location.path('/') }, 10000);
});

mainApp.controller('twitterReviewCtrl',function($scope,$location,$interval){
     $scope.goToOptimize = function(){
         $location.path('/optimize')
     }

     $scope.getMusic = function(){
        $location.path('/music')
    }
    x=0;
    $scope.twitter_messages_4 = [{heading : "rustysteel-1m",message:"I thought this was a music festival.I thought wrong#Qualcomm2018",color:'rgb(210, 81, 70)'},
                                {heading : "chrisbacon-2m",message:"Soooo boring music festival",color:"rgb(150, 106, 175)"},
                                {heading : "chrisbacon-2m",message:"#qualcomm festival 2018 blows",color:"rgb(80, 115, 240)"},
                                {heading : "rustysteel-1m",message:"I thought this was a music festival.I thought wrong#Qualcomm2018",color:"black"}];

    var twitter_messages = [
        {heading : "chrisbacon-2m",message:"Soooo boring music festival",color:"rgb(210, 81, 70)"},
        {heading : "chrisbacon-2m",message:"#qualcomm festival 2018 blows",color:"rgb(150, 106, 175)"},
        {heading : "rustysteel-1m",message:"I thought this was a music festival.I thought wrong#Qualcomm2018",color:"rgb(80, 115, 240)"},
        {heading : "chrisbacon-2m",message:"Soooo boring music festival",color:"black"},
        {heading : "chrisbacon-2m",message:"#qualcomm festival 2018 blows",color:"rgb(210, 81, 70)"},
        {heading : "rustysteel-1m",message:"I thought this was a music festival.I thought wrong#Qualcomm2018",color:"rgb(150, 106, 175)"},
        {heading : "chrisbacon-2m",message:"Soooo boring music festival",color:"rgb(80, 115, 240)"},
        {heading : "chrisbacon-2m",message:"#qualcomm festival 2018 blows",color:"black"},
        {heading : "rustysteel-1m",message:"I thought this was a music festival.I thought wrong#Qualcomm2018",color:"rgb(210, 81, 70)"},
        {heading : "chrisbacon-2m",message:"Soooo boring music festival",color:"rgb(150, 106, 175)"},
    ]
    $interval(function(){
        $scope.twitter_messages_4=[];
        for(var j = 0;j<4;j++){
            if(x > (twitter_messages.length-1)){
                x = 0;
            }
            $scope.twitter_messages_4.push(twitter_messages[x]);
            x++;
        }
        if(x >= 3){
            x = x - 3;  
        }
    },5000)  
 });

mainApp.controller('optimizeCtrl',function($scope,$location){
    $scope.goToDashboard = function(){
        $location.path('/dashboard')
    }

    setTimeout(function(){ $location.path('/') },5000);
});

mainApp.controller('dashboardCtrl',function($scope,$location,getDetails){
    $scope.musicSelected = false;
    $scope.movementsDetected = false;

    //var MusicChartColor = ['rgb(210, 81, 70)','rgb(150, 106, 175)','rgb(80, 115, 240)','rgb(150,150,150)'];
    var MusicChartColor = { 'rock' :'rgb(210, 81, 70)','edm': 'rgb(150, 106, 175)','hiphop':'rgb(80, 115, 240)','pop':'rgb(150,150,150)' }
    $scope.closeDashboard = function(){
        $location.path('/')
        //document.getElementById('sidenav').style.marginLeft = "-40%";
        //document.getElementById("sidenav").style.WebkitTransition = "all 0.5s"; // Code for Safari 3.1 to 6.0
        //document.getElementById("sidenav").style.transition = "all 0.5s";       // Standard syntax
    }
    
	
    getDetails.getMovements().then(function onSuccess(data){
        console.log(data);
        //movementsData = JSON.parse(data);
        console.log(data.data);
        if(!data.data || data.data.data.length == 0){
           //console.log('data');
            $scope.movementsDetected = true;
         }
        else{
            //console.log(data.data.data.length);
            $scope.movementsDetected = false;
                        
            for(var i=0;i<data.data.data.length;i++){
                //console.log(data.data.data[i]);
                var local = moment.utc(data.data.data[i].interval).local().format();
                console.log(local, "- UTC now to local");

                time = data.data.data[i].interval;
               // console.log(time);
                newdate = new Date(local);    
                hour = newdate.getHours();
                minute = newdate.getMinutes();
                let new_minute;
                if(minute == 0){
                    new_minute = minute+'0';
                }
                else{
                    new_minute = minute;
                }
                label = hour + ":" + new_minute;
                movements = data.data.data[i].count;
                dashboard_crowd_Chart.data.labels.push(label);
        
                    dashboard_crowd_Chart.data.datasets.forEach((dataset) => {
                            dataset.data.push(movements);
                            //dataset.data.push(movements);
                        });
                        
                dashboard_crowd_Chart.update();
                  }
                //console.log(dashboard_crowd_Chart.data)
            }
        });
    getDetails.getMusicData().then(function onSuccess(data){
        var total_download = 0;
        //console.log("data of chart",data.data.length);
        if(data.data.length == 0){

            $scope.musicSelected = true; 
        }
        else{

            $scope.musicSelected = false; 
        for(var index=0;index<data.data.length;index++){
           // console.log(data)
            total_download += data.data[index].download;
        }
        
        for(var index=0;index<data.data.length;index++){
            let DashboardData = data.data[index];
            let music_name = DashboardData.musicName;
            let music_download = ((DashboardData.download) / total_download) * 100;
            console.log(Math.round(music_download))
            music_download = Math.round(music_download);
            dashboard_music_chart.data.labels.push(music_name);
            dashboard_music_chart.data.datasets.forEach((dataset) => {
                dataset.data.push(music_download);
                /* let colorindex = index % 4; */
                dataset.backgroundColor.push(MusicChartColor[music_name]);
            });
        }
        dashboard_music_chart.update();
    }
    })

    dashboard_music_chart = new Chart('music_chart',{
        type: 'doughnut',
        data: {
            datasets: [{
                data: [],
                backgroundColor:[],
                
            }],
            labels: [],
        },
        options:{
            pieceLabel: {
                mode: 'value',
                render: function (args) {
                    return args.value + '%';
                  },
                fontColor: function (args) {
                    return (args.dataset.backgroundColor[args.index]);
                  },
                position: 'outside',
                fontSize:14,
                fontStyle:'bold'
            },
            tooltips: {
                enabled : false,
                callbacks: {
                    label: function(tooltipItem, data) {
                    console.log(data);
                    let label = data.labels[tooltipItem.index];
                    let value = data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
                    console.log(label);
                    console.log(value);
                    return [label + ' : ' +  value + '%'];
                    }
                }
            },
            elements: {
                arc: {
                    borderWidth: 0
                }
            },
            legend:{
                display:false,
                position:'right',
                labels:{
                boxWidth:8,
                fontSize:11,
                fontStyle:'bold',
                padding:25,
                 },
            
        }}
    }) 

      

   dashboard_crowd_Chart = new Chart('crowd_chart',{
        type: 'line',
        data: {
            lineColor:"blue",
            labels:[],
            datasets: [
            {
                label: "crowd",
                fill:true,
                backgroundColor : "rgba(102, 140, 255,0.6)",
                data: [],
		lineTension:0,
		borderWidth:1,
		pointRadius : 0,
                borderColor: "rgba(0, 64, 255)",
            },
        ]},
        options :{ 
            legend: {
                display: false,
                position: "top",
                labels: {
                    fontColor: "rgba(0,0,0,0.5)",
                    fontSize: 12
                }
            },
            scales:{
                xAxes: [{
                   /*  type: 'time',
                    time: {
                        unit:'hour',
                        displayFormats: {
                            minute: 'h:mm'
                        }
                    } */
                }],
                yAxes: [{
                    display:true,
                }]
            }}
    }) 
});



//SELECT series.minute as interval,  coalesce(cnt.count,0) as count from (select count(*) as count,to_timestamp(floor((pir_time_stamp/1000)/1800)*1800) as timestamp from music_pir_timestamp group by timestamp) cnt right join(select generate_series(min(date_trunc('hour',to_timestamp(pir_time_stamp/1000))),max(date_trunc('minute',to_timestamp(pir_time_stamp/1000))),'30m') as minute from music_pir_timestamp ) series on series.minute = cnt.timestamp where series.minute>= now()-interval'6 hours' order by series.minute;
