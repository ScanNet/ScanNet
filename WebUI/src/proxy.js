// Using the simplest possible proxying for proxying data located on servers
// can also try express-http-proxy or http-proxy-middleware
//const proxy = require('proxy-middleware');
const proxy = require('express-http-proxy');
const url = require('url');

var proxyRules = [
  { path: '/scans/index', targetUrl: 'http://localhost:5001/index' },
  { path: '/scans/monitor', targetUrl: 'http://localhost:5001' },
//  { path: '/scans/index', targetUrl: 'http://localhost:5001/index' },
  { path: '/solr', targetUrl: 'http://localhost/solr' },
];

module.exports = function() {
  const app = this;

  // Simple proxy rules
  for (var i = 0; i < proxyRules.length; i++) {
    var proxyRule = proxyRules[i];
//  Simple but sometimes add trailing slash due to req.url being enforced to have '/'
//  Problematic for redirecting /scans/index
//    app.use(proxyRule.path, proxy(proxyRule.targetUrl));

    proxyRule.target = url.parse(proxyRule.targetUrl);
    proxyRule.targetRoot = url.resolve(proxyRule.targetUrl, '/');
    proxyRule.targetPath = proxyRule.target.pathname;
    console.log('Proxying: ' + proxyRule.path + ' to ' + proxyRule.targetUrl + ' with '
      + 'host=' + proxyRule.targetRoot + ', path=' + proxyRule.targetPath);
    app.use(proxyRule.path, proxy(proxyRule.targetRoot, {
      forwardPath: function(pr, req, res) {
        var prefix = pr.targetPath;
        var path = prefix + (req.originalUrl === req.baseUrl? '':url.parse(req.url).path);
        path = path.replace('//', '/');
        //console.log('Proxying ' + req.originalUrl + ' to ' + path);
        return path;
      }.bind(this, proxyRule)
    }));
  }
};

