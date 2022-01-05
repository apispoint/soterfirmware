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

const dns  = require('dns');
const http = require('http');

const ping = 'www.google.com';
const host = '192.168.8.1';
const port = 3001;

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
        dns.lookup(ping, function(err){
            var status = {ok : err === null}
            res.statusCode = 200;
            res.end(JSON.stringify(status));
        });
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`Ping Server running at http://${host}:${port}/`);
});