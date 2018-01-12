'use strict';

const authentication = require('feathers-authentication');

const GithubStrategy = require('passport-github').Strategy;
const GithubTokenStrategy = require('passport-github-token');

module.exports = function() {
  const app = this;

  let config = app.get('auth');
  
  config.github.strategy = GithubStrategy;
  config.github.tokenStrategy = GithubTokenStrategy;

  app.set('auth', config);
  app.configure(authentication(config));
};
