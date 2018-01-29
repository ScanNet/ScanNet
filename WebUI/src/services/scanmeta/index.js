'use strict';

const path = require('path');
const NeDB = require('nedb');
const service = require('feathers-nedb');
const hooks = require('./hooks');

// Metadata associated with scans that outlive reindexing attempts
module.exports = function(){
  const app = this;

  const db = new NeDB({
    filename: path.join(app.get('nedb'), 'scanmeta.db'),
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
  app.use('/api/scanmeta', service(options));

  // Get our initialize service to that we can bind hooks
  const scanmetaService = app.service('/api/scanmeta');

  // Set up our before hooks
  scanmetaService.before(hooks.before);

  // Set up our after hooks
  scanmetaService.after(hooks.after);
};
