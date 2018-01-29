'use strict';

const assert = require('assert');
const app = require('../../../src/app');

describe('scanmeta service', function() {
  it('registered the scanmeta service', () => {
    assert.ok(app.service('scanmeta'));
  });
});
