'use script';

(function() {
    var app = angular.module('apSignin', []);

    app.controller("SigninCtrl", ['$scope', '$rootScope', '$http', function($scope, $rootScope, $http) {
        if(!$rootScope.devid || $rootScope.connected !== 'up')
            $rootScope.showconfig = true;

        $scope.uiCreated = function () {
            M.Collapsible.init(document.querySelectorAll('.collapsible'));
        };

        $scope.showsignpasswd = "password";

        $scope.running = false;
        $scope.mfa = false;
        $scope.mfareturn = false;
        $scope.message = 'Signing in automatically links your device';
        $scope.credentials = {
            user: '',
            password: ''
        };
        $scope.passwordconf = null;

        $scope.resetMFA = function() {
            $scope.mfa = false;
            delete $scope.credentials.otp;
        };

        $scope.login = function(auth) {
            $scope.running = true;

            $scope.message = 'Signing in...';
//            if(auth) $scope.message = 'Signing in...';
//            else     $scope.message = 'Signing up...';

            if($scope.mfa)
               $scope.credentials.otp.time = new Date().getTime();

            $scope.mfareturn = false;
            $http.post('http://192.168.8.1:3002?id=' + $rootScope.devid + '&auth' /*+ (auth ? '&auth' : '')*/, angular.toJson($scope.credentials))
            .then(function(response) {
                $scope.message = 'Provisioning...';

                const { data, status } = response;

                if(auth == false && ("otp" in data)) {
                    $scope.otp = {};
                    $scope.otp.qrcode = data.otp.qrcode;
                    $scope.otp.secret = data.otp.secret;

                    $scope.otp.secret =
                        $scope.otp.secret
                        .replace(/(.{4})(.{4})(.{4})(.{4})(.{4})(.{4})(.{4})(.{4})/,
                                 '$1 $2 $3 $4 $5 $6 $7 $8')
                        .replace(/=/g, " ")
                        .trim()
                        .replace(/ /g, " - ");

                    $scope.mfareturn = true;
                }

                $scope.resetMFA();
                if(status == 200) {
                    M.toast({html: 'Provisioning Successful...', classes: 'rounded', displayLength: 3000});
                    $scope.message = 'Done! Check your device status at soter.apispoint.com';
                }
                else {
                                      $scope.message = "Something went wrong. Contact support and list what was Done.";
                    if(status >= 210) $scope.message += "\nDone: Created User";
                    if(status >= 211) $scope.message += "\nDone: Linked Device";
                }
            })
            .catch(function(response) {
                const { data, status } = response;
                // Handle login errors here
//                if(status == 401) {
//                    $scope.mfa = true;
//                    $scope.message = '';
//                }
//                else if(status == 400 || status == 403) {
                if(status >= 400 && status < 500) {
                    $scope.message = 'Invalid credentials';
                    $scope.resetMFA();
                }
                else {
                    M.toast({html: 'Provisioning Failed...', classes: 'rounded', displayLength: 3000});
                    $scope.message = 'Services are unreachable';
                    $scope.resetMFA();
                }
            })
            .finally(function() {
                $scope.running = false;
            });
        };

        $scope.toggleSigninPassword = function(evt) {
            $scope.showsignpasswd = evt.target.checked ? "text" : "password";
        };

        $scope.goBack = function() {
            $scope.mfa = false;
            $rootScope.showconfig = true;
        };
    }]);
})();
