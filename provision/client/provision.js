'use script';

(function() {
    var app = angular.module('apProvision', []);

    app.controller("ProvisionCtrl", ['$scope', '$rootScope', '$http', '$timeout', function($scope, $rootScope, $http, $timeout) {
        $scope.uiCreated = function () {
            M.FormSelect.init(document.querySelectorAll('select'));
        };

        $scope.wifi = {
            sec    : 'psk',
            name   : '',
            passwd : ''
        };

        $scope.disable    = true;
        $scope.showpasswd = "password";

        $scope.atoh = function(str) {
            var hex = "";

            for (var i = 0; i < str.length; i++)
                hex += Number(str.charCodeAt(i)).toString(16);

            return hex;
        }

        $scope.htoa = function(hex) {
            var str = "";

            var hexes = hex.match(/.{2}/g) || [];
            for(var i = 0; i < hexes.length; i++)
                str += String.fromCharCode(parseInt(hexes[i], 16));

            return str;
        }

        $scope.testWiFiConnection = function(cb) {
            $http.get('http://192.168.8.1:3001')
            .then(function(response) {
                const { data, status } = response;
                cb(data.ok);
            })
            .catch(function(response) {
                cb(false);
            });
        }

        $http.get('http://192.168.8.1:3000')
        .then(function(response) {
            const { data, status } = response;

            $scope.wifi      = data;
            $scope.wifi.name = $scope.wifi.ssid ? $scope.htoa($scope.wifi.ssid) : $scope.wifi.name;
            $scope.wifiprev  = JSON.parse(JSON.stringify($scope.wifi));
        })
        .catch(function(response) {
            M.toast({html: 'Unable to get Wi-Fi configuration...', classes: 'rounded', displayLength: 3000});
        })
        .finally(function() {
            $scope.disable = false;
        });

        $scope.testWiFiConnection(function(ok) {
            $scope.completeConfigWifi(ok, false);
        });

        $scope.completeConfigWifi = function(isConnected, autonext) {
            $rootScope.connected = isConnected ? "up" : "down";
            $rootScope.getDeviceInformation();
            $scope.disable = false;

            if(isConnected)
                $scope.wifiprev = JSON.parse(JSON.stringify($scope.wifi));

            if(isConnected && autonext)
                $timeout(() => { $rootScope.showconfig = false; }, 3000, true);
        }

        $scope.configWifi = function(renew) {
            $rootScope.ip4 = "...";
            $rootScope.ip6 = "...";

            $scope.disable       = true;
            $rootScope.connected = "testing...";

            var params =
                renew
                    ? 'renew'
                    : 'ssid='    + $scope.atoh($scope.wifi.name) +
                      '&sec='    + $scope.wifi.sec +
                      '&passwd=' + $scope.wifi.passwd;

            if(!renew)
                $scope.wifiprev = null;

            $http.post('http://192.168.8.1:3000?' + params)
            .then(function(response) {
                $scope.testWiFiConnection(function(ok) {
                    $scope.completeConfigWifi(ok, !renew);
                });
            })
            .catch(function(response) {
                M.toast({html: 'Failed to configure Wi-Fi...', classes: 'rounded', displayLength: 3000});
                $scope.completeConfigWifi(false, false);
            });
        };

        $scope.togglePassword = function(evt) {
            $scope.showpasswd = evt.target.checked ? "text" : "password";
        };
        $scope.nextProvision = function() {
            $rootScope.showconfig = false;
        };
    }]);
})();
