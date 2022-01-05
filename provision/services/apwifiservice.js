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
const http = require('http');
const url  = require('url');
const proc = require('child_process');

const host = '192.168.8.1';
const port = 3005;

const aphostfile = '/etc/hostapd.conf';
const pswd_regex = /^wpa_passphrase=(.*)$/gm;

const restart = function() {
    proc.exec(`systemctl restart bb-wl18xx-wlan0`, function(error){});
};

const server = http.createServer((req, res) => {
    res.setHeader('Access-Control-Allow-Origin',  '*');
    res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, POST, GET');
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
    else if(req.method === "POST") {
        var query = url.parse(req.url, true).query;

        var passwd = query.passwd || '';

        if(!passwd) {
            res.statusCode = 500;
            res.end();
            return;
        }

        proc.exec(`sed -i 's/^wpa_passphrase=.*$/wpa_passphrase=${passwd}/gI' ${aphostfile}`, function(error){
            if(!error) res.statusCode = 200;
            else       res.statusCode = 408;

            res.end(res.statusCode == 200 ? restart : null);
        });
    }
    else if(req.method === "GET") {
        var resp = {
            passwd : ''
        };

        if(fs.existsSync(aphostfile)) {
            try {
                var data = fs.readFileSync(aphostfile, 'utf8');

                pswd_regex.lastIndex = 0;
                var passwd = pswd_regex.exec(data);

                if(passwd != null) resp.passwd = passwd[1];
            } catch(e) {}
        }

        res.statusCode = 200;
        res.end(JSON.stringify(resp));
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`AP Wifi Configuration Server running at http://${host}:${port}/`);
});