'use script';

(function() {
  var app = angular.module('apCalibrate', []);

  app.controller("CalibrateCtrl", ['$scope', '$rootScope', '$http', function($scope, $rootScope, $http) {
    $scope.createPlotly = function () {
      Plotly.purge('calDiv');

      var trace1_3d = {
        x: [],
        y: [],
        z: [],
        mode: 'markers',
        marker: {
          size: 2,
          line: {
            color: 'rgba(217, 217, 217, 0.14)',
            width: 0.5
          },
          opacity: 0.8
        },
        type: 'scatter3d'
      };
      var data = [trace1_3d];
      var layout = {
        dragmode: "orbit",
        showlegend:false,
        paper_bgcolor: 'rgba(0,0,0,0)',
        plot_bgcolor: 'rgba(0,0,0,0)',
        margin: {
          l: 0,
          r: 0,
          b: 0,
          t: 0
      }};
      Plotly.newPlot('calDiv', data, layout, {displayModeBar: false});
    };

    $scope.running = false;
    $scope.waiting = "--";

    $scope.ox = 0;
    $scope.oy = 0;
    $scope.oz = 0;

    $http.get('http://192.168.8.1:3006?offset').then(function(response) {
        const { data, status } = response;
        $scope.ox = data.mag_offsets[0];
        $scope.oy = data.mag_offsets[1];
        $scope.oz = data.mag_offsets[2];
    });

    $scope.calibrate = function() {
        $scope.createPlotly();

        $scope.running = true;
        var idx = 0;
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "http://192.168.8.1:3006");

        xhr.onreadystatechange = function () {
            if(xhr.readyState === 4)
                $scope.$apply(function () { $scope.running = false; });
        };
        xhr.onprogress = function () {
            var cidx = xhr.responseText.length;
            if(idx == cidx)
                return;

            var newdata = xhr.responseText.substring(idx, cidx).trim();
            idx = cidx;

            var segs = newdata.trim().split('\n');
            for(var i = 0; i < segs.length; i++) {
                if(!segs[i].startsWith('>')) {
                    var nums = segs[i].split(',');
                    Plotly.extendTraces('calDiv', {
                      x: [[parseInt(nums[0])]],
                      y: [[parseInt(nums[1])]],
                      z: [[parseInt(nums[2])]]
                    }, [0]);
                } else {
                    var nums = segs[i].substring(1).split(',');
                    $scope.$apply(function () {
                        $scope.ox = nums[0];
                        $scope.oy = nums[1];
                        $scope.oz = nums[2];
                    });
                }
            }
        };

        xhr.send();
    };
  }]);
})();
