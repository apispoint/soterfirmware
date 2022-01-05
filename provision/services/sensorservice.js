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

const fs   = require('fs');
const proc = require('child_process');
const http = require('http');
const url  = require('url');

const host = '192.168.8.1';
const port = 3007;

const tmp1_regex = /^TempC:\s+(-?\d+.\d+|nan)/g;

const gps1_regex = /^\$(?:GPGGA|GPRMC),/mg;
const gps2_regex = /(?:lon|lat|alt): (-?\d+\.\d+|nan)/mg;
const gps3_regex = /^time\(GPS\):\s+(\d+)/mg;

const mag1_regex = /Failed|Usage/mg;
const mag2_regex = /angle:\s+(\d+.\d+|nan)/mg;

const sps1_regex = /SPS_C: (\d+)/mg;

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

        if('tmp' in query) {
            proc.exec(`/opt/apispoint/soter/module.sh tmp`, (error, stdout, stderr) => {
                var out = stdout.toString().trim() || "";

                tmp1_regex.lastIndex = 0;
                var cdeg = parseFloat(tmp1_regex.exec(out)[1]);

                var tmp = {ok: !!cdeg && !error, tmp: cdeg};
                res.statusCode = 200;

                res.end(JSON.stringify(tmp));
            });
        }
        else if('gps' in query) {
            proc.exec(`/opt/apispoint/soter/module.sh gps`, (error, stdout, stderr) => {
                var out = stdout.toString().trim() || "";

                gps1_regex.lastIndex = 0;
                var gps = {ok: out.match(gps1_regex) !== null && !error, tuple: []};
                res.statusCode = 200;

                if(gps.ok) {
                    gps2_regex.lastIndex = 0;
                    while(match = gps2_regex.exec(out))
                        gps.tuple.push(parseFloat(match[1]));

                    gps3_regex.lastIndex = 0;
                    gps.tuple.push(parseFloat(gps3_regex.exec(out)[1]));
                }

                res.end(JSON.stringify(gps));
            });
        }
        else if('mag' in query) {
            var arg = "0 0 0";

            try {
                var out = JSON.parse(fs.readFileSync('/opt/apispoint/config/mag_calibration.json', 'utf8'));
                arg = `${out.mag_offsets[0]} ${out.mag_offsets[1]} ${out.mag_offsets[2]}`;
            } catch(err) {}

            proc.exec(`/opt/apispoint/soter/module.sh mag "${arg}"`, (error, stdout, stderr) => {
                var out = stdout.toString().trim() || "";

                mag1_regex.lastIndex = 0;
                var mag = {ok: out.match(mag1_regex) === null && !error};
                res.statusCode = 200;

                if(mag.ok) {
                    mag2_regex.lastIndex = 0;
                    mag.mag = parseFloat(mag2_regex.exec(out)[1]);
                }

                res.end(JSON.stringify(mag));
            });
        }
        else if('sps' in query) {
            proc.exec(`/opt/apispoint/soter/module.sh sps 3`, (error, stdout, stderr) => {
                var out = stdout.toString().trim() || "";

                sps1_regex.lastIndex = 0;
                var match = out.match(sps1_regex);
                var sps = {ok: match.length == 2 && !error};
                res.statusCode = 200;

                if(sps.ok)
                    sps.sps = parseInt(match[1].split(' ')[1]);

                res.end(JSON.stringify(sps));
            });
        }
        else if('mic' in query) {
            res.setHeader('Content-Type', 'text/csv');
            res.setHeader('Transfer-Encoding', 'chunked');
            res.statusCode = 200;

            var pid = proc.spawn('/opt/apispoint/soter/module.sh', ['rec 3'], { "detached": true });

            pid.stdout.on('data', function (data) {
                res.write(data.toString());
            });

            pid.stderr.on('data', function (data) {
            });

            pid.on('exit', function (code) {
                res.end();
            });
        }
        else
            res.end();
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`Sensor Server running at http://${host}:${port}/`);
});