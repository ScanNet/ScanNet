'use strict';

const assert = require('assert');
const app = require('../../../src/app');

describe('scan service', function() {
  it('registered the scans service', () => {
    assert.ok(app.service('scans'));
  });
});
