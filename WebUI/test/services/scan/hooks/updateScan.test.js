'use strict';

const assert = require('assert');
const updateScan = require('../../../../src/services/scan/hooks/updateScan.js');

describe('scan updateScan hook', function() {
  it('hook can be used', function() {
    const mockHook = {
      type: 'after',
      app: {},
      params: {},
      result: {},
      data: {}
    };

    updateScan()(mockHook);

    assert.ok(mockHook.updateScan);
  });
});
