var winston = require('winston');
var expressWinston = require('express-winston');
var WinstonContext = require('winston-context');

var expressLogger = expressWinston.logger({
  transports: [
    new (winston.transports.File)({ filename: 'logs/express.log' })
  ],
  meta: true, // optional: control whether you want to log the meta data about the request (default to true)
  msg: 'HTTP {{res.statusCode}} {{req.method}} {{res.responseTime}}ms {{req.url}}', // optional: customize the default logging message.
  //expressFormat: true, // Use the default Express/morgan request formatting, with the same colors. Enabling this will override any msg and colorStatus if true. Will only output colors on transports with colorize set to true
  //colorStatus: true, // Color the status code, using the Express/morgan color palette (default green, 3XX cyan, 4XX yellow, 5XX red). Will not be recognized if expressFormat is true
  //ignoreRoute: function (req, res) { return false; } // optional: allows to skip some log messages based on request and/or response
});

var defaultLogger = new winston.Logger({
  level: 'info',
  transports: [
    new (winston.transports.Console)({handleExceptions: true}),
    new (winston.transports.File)({ filename: 'logs/server.log', handleExceptions: true })
  ],
  exitOnError: false
});
// winston.add(winston.transports.File, {
//   filename: 'logs/server.log',
//   handleExceptions: true
// });
// winston.exitOnError = false;

var Logger = function(name, meta) {
  var logger = winston.loggers.get(name) || defaultLogger;
  meta = meta || {};
  if (logger === winston && !meta.name) { meta.name = name; }
  var ctx = new WinstonContext(logger, '', meta);
  ctx.expressLogger = expressLogger;
  return ctx;
};

module.exports = Logger;
