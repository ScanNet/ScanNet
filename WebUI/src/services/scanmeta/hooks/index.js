'use strict';

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
  find: [],
  get: [],
  create: [],
  update: [],
  patch: [],
  remove: []
};
