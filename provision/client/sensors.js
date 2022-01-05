'use script';

(function() {
  var app = angular.module('apSensors', []);

  var GPS_NOSIG = "no sat";

  app.controller("SensorsCtrl", ['$scope', '$rootScope', '$http', '$window', function($scope, $rootScope, $http, $window) {
    $scope.uiCreated = function () {
        M.Collapsible.init(document.querySelectorAll('.collapsible'));
    };

    $scope.updatestatus= "";

    $scope.updating = false;

    //
    // Firmware Tasking
    //

    $scope.fwTask = function(revert) {
        $scope.updatestatus="";
        $scope.updating = true;
        $scope.rebooting = false;

        verb = revert ? 'restore' : 'update';

        $http.post('http://192.168.8.1:3008?' + (revert ? "revert" : "update"))
        .then(function(response) {
            const { data, status } = response;
            $scope.updatestatus=data.message;
        })
        .catch(function(err) {
            if(err.status < 0 && err.xhrStatus == "error") {
                $scope.rebooting = true;
                $scope.updatestatus = "Rebooting to complete....\nRefresh the page when SO+ER is back up.";
            }
            else $scope.updatestatus="Error during " + verb;
        })
        .finally(function() {
            if(!$scope.rebooting)
                $rootScope.getDeviceInformation();

            $scope.updating = false;
        });
    };

    //
    // Temperature Module
    //

    $scope.tmpRunning = false;

    $scope.tmpc = "--";
    $scope.tmpf = "--";

    $scope.tmpButton = function() {
        $scope.tmpc = "--";
        $scope.tmpf = "--";
        $scope.tmpRunning = true;
        $http.get('http://192.168.8.1:3007?tmp')
        .then(function(response) {
            const { data, status } = response;
            $scope.tmpc = data.ok ? data.tmp.toFixed(1)                     : "BAD";
            $scope.tmpf = data.ok ? (($scope.tmpc * 1.8) + 32.0).toFixed(1) : "BAD";
        })
        .catch(function() {
            $scope.tmpc = "BAD";
            $scope.tmpf = "BAD";
        })
        .finally(function() {
            $scope.tmpRunning = false;
        });
    };

    //
    // GPS Module
    //

    $scope.gpsRunning = false;

    $scope.lon = "--";
    $scope.lat = "--";
    $scope.alt = "--";
    $scope.tim = "--";

    $scope.gpsButton = function() {
        $scope.lon = "--";
        $scope.lat = "--";
        $scope.alt = "--";
        $scope.tim = "--";
        $scope.gpsRunning = true;
        $http.get('http://192.168.8.1:3007?gps')
        .then(function(response) {
            const { data, status } = response;
            if(data.ok) {
                $scope.lon = !!data.tuple[0] ? data.tuple[0].toFixed(5) : GPS_NOSIG;
                $scope.lat = !!data.tuple[1] ? data.tuple[1].toFixed(5) : GPS_NOSIG;
                $scope.alt = !!data.tuple[2] ? data.tuple[2].toFixed(0) : GPS_NOSIG;
                $scope.tim = !!data.tuple[3] ? data.tuple[3].toFixed(0) : GPS_NOSIG;
            }
            else {
                $scope.lon = "BAD";
                $scope.lat = "BAD";
                $scope.alt = "BAD";
                $scope.tim = "BAD";
            }
        })
        .catch(function() {
            $scope.lon = "BAD";
            $scope.lat = "BAD";
            $scope.alt = "BAD";
            $scope.tim = "BAD";
        })
        .finally(function() {
            $scope.gpsRunning = false;
        });
    };

    //
    // Magnetometer Module
    //

    $scope.magRunning = false;

    $scope.mag = "--";

    $scope.magButton = function() {
        $scope.mag = "--";
        $scope.magRunning = true;
        $http.get('http://192.168.8.1:3007?mag')
        .then(function(response) {
            const { data, status } = response;
            $scope.mag = data.ok ? data.mag.toFixed(0) : "BAD";
        })
        .catch(function() {
            $scope.mag = "BAD";
        })
        .finally(function() {
            $scope.magRunning = false;
        });
    };

    //
    // Microphone Module
    //

    $scope.createMicPlotly = function(data) {
      var trace1 = {
        type: "scatter",
        mode: "lines",
        name: 'Mic 1',
        y: data ? data[0] : [],
        line: {color: '#7293cb'}
      }
      var trace2 = {
        type: "scatter",
        mode: "lines",
        name: 'Mic 2',
        y: data ? data[1] : [],
        line: {color: '#e1974c'}
      }
      var trace3 = {
        type: "scatter",
        mode: "lines",
        name: 'Mic 3',
        y: data ? data[2] : [],
        line: {color: '#84ba5b'}
      }
      var trace4 = {
        type: "scatter",
        mode: "lines",
        name: 'Mic 4',
        y: data ? data[3] : [],
        line: {color: '#d35e60'}
      }
      var data = [trace1,trace2,trace3,trace4];
      var layout = {
        showlegend:true,
        paper_bgcolor: 'rgba(0,0,0,0)',
        plot_bgcolor: 'rgba(0,0,0,0)',
        margin: {
          l: 50,
          r: 10,
          b: 30,
          t: 10
      }};
      Plotly.newPlot('micDiv', data, layout, {displayModeBar: false});
    };

    $scope.spsRunning = false;
    $scope.micRunning = false;

    $scope.sps = "--";

    $scope.trace = function() {
        $scope.micRunning = true;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "http://192.168.8.1:3007?mic");

        var data;

        xhr.onreadystatechange = function () {
            if(xhr.readyState === 4 && data) {
                var trace = [[],[],[],[]];

                var lines = data.trim().split('\n');
                for(var i = 0; i < lines.length; i++) {
                    var samples = lines[i].split(',');
                    for(var j = 0; j < samples.length; j++)
                        trace[j].push(samples[j]);
                }

                $scope.createMicPlotly(trace);
                $scope.$apply(function () { $scope.micRunning = false; });
            }
        };
        xhr.onprogress = function () {
            data = xhr.responseText;
        };
        xhr.onerror = function () {
            $scope.$apply(function () { $scope.micRunning = false; });
        };
        xhr.ontimeout = function (e) {
            $scope.$apply(function () { $scope.micRunning = false; });
        };

        xhr.send();
    };

    $scope.micButton = function() {
        $scope.sps = "--";
        Plotly.deleteTraces('micDiv', [0,1,2,3]);
        $scope.spsRunning = true;

        $http.get('http://192.168.8.1:3007?sps')
        .then(function(response) {
            const { data, status } = response;
            if(data.ok) {
                $scope.sps = ((data.sps / 10).toFixed() * 10).toLocaleString();
                $scope.trace();
            }
            else $scope.sps = "BAD";
        })
        .catch(function() {
            $scope.sps = "BAD";
        })
        .finally(function() {
            $scope.spsRunning = false;
        });
    };

    $scope.sleep = function(time) {
      return new Promise((resolve) => setTimeout(resolve, time));
    };

    $scope.resizePlotly = function(delay) {
      $scope.sleep(delay ? 50 : 0).then(() => {
        Plotly.relayout('micDiv', {
          width: document.getElementById("micDivRow").offsetWidth
        });
      });
    };

    $window.addEventListener("resize", function() { $scope.resizePlotly(); }, true);
  }]);
})();
