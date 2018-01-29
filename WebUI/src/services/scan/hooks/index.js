'use strict';

const updateScan = require('./updateScan');

const globalHooks = require('../../../hooks');
const hooks = require('feathers-hooks');

exports.before = {
  all: [],
  find: [hooks.removeQuery('_'), globalHooks.rewriteQuery()],
  get: [],
  create: [],
  update: [],
  patch: [],
  remove: []
};

exports.after = {
  all: [],
  find: [hooks.remove('files', '_id', '_orig'), updateScan()],
  get: [hooks.remove('_id', '_orig'), updateScan()],
  create: [],
  update: [],
  patch: [hooks.remove('_id'), updateScan()],
  remove: []
};
