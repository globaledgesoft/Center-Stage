mainApp.service('getDetails',function($http){
    return {
        getMusicData : function(){
            var req = {
                method: 'GET',
                url: "/getMusicData"
            }
            return $http(req).then(function(response) {
                return response.data;
            });
        },
        getMovements : function(){
            var req = {
                method:'GET',
                url:'/getMovements'
            }
            return $http(req).then(function(response){
                return response;
            });
        },
        singMusic : function(data){
            var test = {music_type:data};
            var post_data = JSON.stringify(test)
            console.log(post_data)
            var req = {
                method: 'POST', 
                url: "/singMusic",
                data : post_data,
                headers: {
                    'Content-Type': 'application/json; charset=utf-8'
                }
            }
            return $http(req).then(function(response) {
                return response.data;
            });
        },
        getCrowdDetails : function(){
            var req = {
                method: 'GET',
                url: "/getCrowdDetails"
            }
            return $http(req).then(function(response) {
                return response.data;
            });
        },
        addMusic : function(id,name){
            var test = {music_id:id,music_name:name};
            var post_data = JSON.stringify(test)
            console.log(post_data)
            var req = {
                method: 'POST', 
                url: "/addMusic",
                data : post_data,
                headers: {
                    'Content-Type': 'application/json; charset=utf-8'
                }
            }
            return $http(req).then(function(response) {
                return response.data;
            });
        }
    }
})

mainApp.factory('socket', function ($rootScope) {
    var socket = io.connect("http://192.168.10.169:8000");
    return {
      on: function (eventName, callback) {
        socket.on(eventName, function () {  
          var args = arguments;
          $rootScope.$apply(function () {
            callback.apply(socket, args);
          });
        });
      }
    };
  });
