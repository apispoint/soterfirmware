//
//    Copyright (c) 2021 APIS Point, LLC.
//    All rights reserved.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

const fs      = require('fs');
const proc    = require('child_process');
const http    = require('http');
const url     = require('url');
const process = require('process');

const host = '192.168.8.1';
const port = 3006;

const server = http.createServer((req, res) => {
    res.setHeader('Access-Control-Allow-Origin',  '*');
    res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, GET');
    res.setHeader('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');

    res.setHeader('Cache-Control', 'private, no-cache, no-store, must-revalidate');
    res.setHeader('Expires',       '-1');
    res.setHeader('Pragma',        'no-cache');

    res.setHeader('Content-Type', 'application/json');

    res.statusCode = 500;

    if(req.method === "OPTIONS") {
        res.statusCode = 200;
        res.end();
    }
    else if(req.method === "GET") {
        var query = url.parse(req.url, true).query;
        if('offset' in query) {
            fs.readFile('/opt/apispoint/config/mag_calibration.json', function read(err, data) {
                res.statusCode = 200;
                res.end(!err ? data.toString() : '{"mag_offsets":[0,0,0]}');
            });
        }
        else {
            res.setHeader('Content-Type', 'text/csv');
            res.setHeader('Transfer-Encoding', 'chunked');
            res.statusCode = 200;

            var pid = proc.spawn('/opt/apispoint/soter/module.sh', ['magcal cal'], { "detached": true });

            pid.stdout.on('data', function (data) {
                res.write(data.toString());
            });

            pid.stderr.on('data', function (data) {
                res.end();
                process.kill(-pid.pid, 'SIGINT');
            });

            pid.on('exit', function (code) {
                res.end();
            });

            setTimeout(function(){ process.kill(-pid.pid, 'SIGINT'); }, 45000);
        }
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`MagCal Server running at http://${host}:${port}/`);
});