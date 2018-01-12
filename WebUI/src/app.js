'use strict';

const path = require('path');
const serveStatic = require('feathers').static;
const favicon = require('serve-favicon');
const compress = require('compression');
const cors = require('cors');
const feathers = require('feathers');
const configuration = require('feathers-configuration');
const hooks = require('feathers-hooks');
const rest = require('feathers-rest');
const bodyParser = require('body-parser');
const socketio = require('feathers-socketio');
const middleware = require('./middleware');
const services = require('./services');
const proxy = require('./proxy');
const routes = require('./routes');
const process_queue = require('./process_queue');

const app = feathers();
app.locals._ = require('lodash');

// View template configuration
app.set('views', path.join(__dirname, '../views'));
app.set('view engine', 'jade');

app.configure(configuration(path.join(__dirname, '..')));

app.use(compress())
  .options('*', cors())
  .use(cors())
  .use(favicon( path.join(app.get('public'), 'favicon.ico') ))
  .use('/', serveStatic( app.get('public') ))
  .configure(proxy)         // proxy routes (need to go before bodyParser
  .use(bodyParser.json({ limit: '50mb' }))
  .use(bodyParser.text({ type: 'text/*', limit: '10mb' }))
  .use(bodyParser.urlencoded({ extended: true }))
  .configure(hooks())
  .configure(rest())
  .configure(socketio())
  .configure(services)
  .configure(process_queue) // Scan process queue (goes after bodyParser, before our routes)
  .configure(routes)        // Our custom routes (goes after bodyParser)
  .configure(middleware);   // Middleware comes last

module.exports = app;
