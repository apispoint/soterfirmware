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
const port = 3000;

const wififile = '/var/lib/connman/soterwifi.config';

const name_regex   = /^Name=(.*)$/gm;
const ssid_regex   = /^SSID=(.*)$/gm;
const sec_regex    = /^Security=(.*)$/gm;
const passwd_regex = /^Passphrase=(.*)$/gm;

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

        if(!('renew' in query)) {
            var ssid   = query.ssid   || '';
            var sec    = query.sec    || 'psk';
            var passwd = query.passwd || '';

            proc.execSync(`bash wificonfigure.sh "${ssid}" ${sec} "${passwd}"`);
        }

        if('renew' in query) {
            proc.execSync(`bash wificonfigure.sh`);
            proc.execSync(`dhclient wlan0 > /dev/null 2>&1`);
        }

        res.statusCode = 200;
        res.end();
    }
    else if(req.method === "GET") {
        var resp = {
            ssid   : '',
            sec    : 'psk',
            passwd : ''
        };

        if(fs.existsSync(wififile)) {
            try {
                var data = fs.readFileSync(wififile, 'utf8');

                name_regex.lastIndex   = 0;
                ssid_regex.lastIndex   = 0;
                sec_regex.lastIndex    = 0;
                passwd_regex.lastIndex = 0;

                var wifi_name   = name_regex.exec(data);
                var wifi_ssid   = ssid_regex.exec(data);
                var wifi_sec    = sec_regex.exec(data);
                var wifi_passwd = passwd_regex.exec(data);

                if(wifi_name   != null) resp.name   = wifi_name[1];
                if(wifi_ssid   != null) resp.ssid   = wifi_ssid[1];
                if(wifi_sec    != null) resp.sec    = wifi_sec[1];
                if(wifi_passwd != null) resp.passwd = wifi_passwd[1];
            } catch(e) {}
        }

        res.statusCode = 200;
        res.end(JSON.stringify(resp));
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`Wifi Configuration Server running at http://${host}:${port}/`);
});
