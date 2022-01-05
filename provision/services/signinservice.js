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

const fs    = require('fs');
const http  = require('http');
const https = require('https');
const url   = require('url');
const qstr  = require('querystring');

const host = '192.168.8.1';
const port = 3002;

const tokenFile = '/opt/apispoint/config/token.sub';

var asset_options = {
    host     : 'api-soter.apispoint.com',
    port     : 443,
    path     : '',
    pathp    : '/asset?id=',
    method   : 'put'
};

var signin_options = {
    host     : 'api-soter.apispoint.com',
    port     : 443,
    path     : '',
    pathp    : '/signin',
    method   : 'post'
};

function httpRequest(options, data, protocol) {
    return new Promise((resolve, reject) => {
        var req = protocol.request(options, function(res) {
            var body = [];
            res.on('data', function(chunk) {
                body.push(chunk);
            });
            res.on('end', function() {
                try {
                    body = JSON.parse(Buffer.concat(body).toString());
                } catch(e) {
                    return reject({"statusCode":500,"body":{}});
                }

                if (res.statusCode < 200 || res.statusCode >= 300) {
                    return reject({"statusCode":res.statusCode,"body":body});
                }
                return resolve({"statusCode":res.statusCode,"body":body});
            });
        });

        if(data)
            req.write(data);

        req.end();
    });
}

const server = http.createServer();
server.on('request', async (req, res) => {
    res.setHeader('Access-Control-Allow-Origin',  '*');
    res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, POST');
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
        let body = [];

        req.on('data', function(chunk) {
            body.push(chunk);
        });
        req.on('end', async function() {
            body = Buffer.concat(body).toString();

            var data = null;
            signin_options.path = signin_options.pathp;
            try {
                let token = await httpRequest(signin_options, body, https);
                res.statusCode = 210;
                data = JSON.stringify(token.body);

                asset_options.path = asset_options.pathp + query.id;
                asset_options.headers = {'Authorization' : 'Bearer ' + token.body.token};
                let devtk = await httpRequest(asset_options, null, https);
                res.statusCode = 211;

                fs.writeFileSync(tokenFile, `${devtk.body[query.id]}`);
                res.statusCode = 200;

                if(data)
                    res.write(data);
            } catch(e) {
                res.statusCode = 500;
            }
            res.end();
        });
    }
    else
        res.end();
});

server.listen(port, host, () => {
    console.log(`Signin Proxy Server running at http://${host}:${port}/`);
});
