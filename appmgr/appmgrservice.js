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

const http = require('http');
const url  = require('url');
const proc = require('child_process');

const host = '192.168.8.1';
const port = 3008;

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

        var output = { message: "" };
        try {
            var out;
            if('revert' in query) out = proc.execSync(`/opt/apispoint/fw/setfw.sh`);
            if('update' in query) out = proc.execSync(`/opt/apispoint/appmgr/appmgr.sh`);

            out = out.toString().split('\n');
            output.message = out[out.length - 2];

            res.statusCode = 200;
        } catch(err) {}

        res.end(JSON.stringify(output));
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`AppMgr Server running at http://${host}:${port}/`);
});
