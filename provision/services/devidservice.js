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

const os   = require('os');
const fs   = require('fs');
const http = require('http');
const proc = require('child_process');

const host = '192.168.8.1';
const port = 3004;
const regx = /(\d+)/g;

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
        var device = { os: "BAD", sw: "BAD", mac: "N/A", ip4: "N/A", ip6: "N/A"};
        try {
            var sos = fs.readFileSync("/etc/soteros.id",      "utf8");
            var ssw = fs.realpathSync("/opt/apispoint/soter", "utf8");

            regx.lastIndex = 0;
            device.os = regx.exec(sos)[1];

            regx.lastIndex = 0;
            device.sw = regx.exec(ssw)[1];

            device.id = proc.execSync(`hexdump -e '8/1 "%c"' /sys/bus/i2c/devices/0-0050/eeprom -s 16 -n 12`).toString();
            device.mac = proc.execSync(`cat /sys/class/net/wlan0/address`).toString().trim();

            var wlan0 = os.networkInterfaces().wlan0;
            for(var i in wlan0) {
                switch(wlan0[i].family) {
                    case 'IPv4':
                        device.ip4 = wlan0[i].address;
                        break;
                    case 'IPv6':
                        device.ip6 = wlan0[i].address;
                        break;
                }
            }
            res.statusCode = 200;
        } catch(err) {
            res.statusCode = 408;
        };
        res.end(JSON.stringify(device));
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`DevID Server running at http://${host}:${port}/`);
});
