'use strict';

// Add any common hooks you want to share across services in here.
//
// Below is an example of how a hook is written and exported. Please
// see http://docs.feathersjs.com/hooks/readme.html for more details
// on hooks.

function normalizeValue(v) {
  if (typeof v === 'string') {
    var vl = v.toLowerCase();
    if (vl === 'true') {
      return true;
    } else if (vl === 'false') {
      return false;
    }
  }
  return v;
}

exports.rewriteQuery = function(options) {
  const rewriteQueries = data => {
    for (let k in data) {
      var value = data[k];
      if (value['$exists'] != undefined) {
        value['$exists'] = normalizeValue(value['$exists']);
      }
      if (value['$regex'] != undefined) {
        if (typeof value['$regex'] === 'string') {
          value['$regex'] = new RegExp(value['$regex']);
        }
      }
    }
  };

  const callback = () => true;

  return function (hook) {
    if (hook.type === 'after') {
      throw new errors.GeneralError(`Provider '${hook.params.provider}' can not rewrite query params on after hook.`);
    }
    const result = hook.params.query;
    const next = condition => {
      if (result && condition) {
        rewriteQueries(result);
      }
      return hook;
    };

    const check = callback(hook);

    return check && typeof check.then === 'function'
      ? check.then(next) : next(check);
  };
};
