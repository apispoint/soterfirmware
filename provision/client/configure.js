'use script';

(function() {
    var app = angular.module('apConfigure', []);

    app.controller("ConfigureCtrl", ['$scope', '$http', function($scope, $http) {
        $scope.appasswords = {
            wifi : '',
            conf : ''
        }

        $scope.showpasswd = "password";

        $http.get('http://192.168.8.1:3005')
        .then(function(response) {
            const { data, status } = response;
            $scope.appasswords.wifi = data.passwd;
        })
        .catch(function(response) {
            M.toast({html: 'Unable to get Wi-Fi AP password...', classes: 'rounded', displayLength: 3000});
        });

        $scope.configAP = function() {
            M.toast({html: 'Wi-Fi AP rebooting...', classes: 'rounded', displayLength: 3000});
            $http.post('http://192.168.8.1:3005?passwd=' + $scope.appasswords.wifi)
            .then(function(response) {
                const { data, status } = response;
            })
            .catch(function(response) {
            });
        };

        $scope.toggleAPPassword = function(evt) {
            $scope.showpasswd = evt.target.checked ? "text" : "password";
        };
    }]);
})();
