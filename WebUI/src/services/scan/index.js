'use strict';

const path = require('path');
const NeDB = require('nedb');
const service = require('feathers-nedb');
const hooks = require('./hooks');

module.exports = function(){
  const app = this;

  const db = new NeDB({
    filename: path.join(app.get('nedb'), 'scans.db'),
    autoload: true
  });

  let options = {
    Model: db,
    paginate: {
      default: 5000,
      max: 10000
    }
  };

  // Initialize our service with any options it requires
  app.use('/api/scans', service(options));

  // Get our initialize service to that we can bind hooks
  const scanService = app.service('/api/scans');

  // Set up our before hooks
  scanService.before(hooks.before);

  // Set up our after hooks
  scanService.after(hooks.after);
};
