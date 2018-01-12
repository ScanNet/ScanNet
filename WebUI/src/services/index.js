'use strict';
const scanmeta = require('./scanmeta');
const scan = require('./scan');
const authentication = require('./authentication');
const user = require('./user');

module.exports = function() {
  const app = this;


  app.configure(authentication);
  app.configure(user);
  app.configure(scan);
  app.configure(scanmeta);
};
