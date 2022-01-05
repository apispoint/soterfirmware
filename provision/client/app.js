'use script';

(function() {
    var app = angular.module('apApplication', ['ngDomEvents', 'apProvision', 'apSignin', 'apConfigure', 'apCalibrate', 'apSensors']);

    app.run(function($rootScope) {
        M.AutoInit();
        $rootScope.cyear = 2020;
        $rootScope.connected  = "testing...";
        $rootScope.showconfig = true;
    });
    app.controller("ApplicationCtrl", ['$scope', '$rootScope', '$http', function($scope, $rootScope, $http) {
        $rootScope.getDeviceInformation = function() {
            $http.get('http://192.168.8.1:3004')
            .then(function(response) {
                const { data, status } = response;
                $rootScope.devid = data.id;
                $rootScope.devos = data.os;
                $rootScope.devsw = data.sw;

                $rootScope.mac = data.mac.toUpperCase();
                $rootScope.ip4 = data.ip4;
                $rootScope.ip6 = data.ip6.toUpperCase();
            })
            .catch(function(response) {
                M.toast({html: 'Cannot obtain Device ID...', classes: 'rounded', displayLength: 3000});
            });
        }
        $rootScope.getDeviceInformation();
    }]);
})();
