const rp = require('request-promise-native');
const Queue = require('./taskqueue');
const _ = require('lodash');

const processUrl = 'http://localhost:5000/process/';
const processBasicFields = ['overwrite', 'from', 'actions'];
const processFields = ['timestamp'].concat(processBasicFields);
const debug = false;

// Queue of scans to be processed
// Persistent storage using sqlite
const store = Queue.getCachedSqlStore({
  type: 'sql',
  dialect: 'sqlite',
  path: './data/process-queue.db'
});
const queue = new Queue(function (task, cb) {
  var url = processUrl + task.id;
  if (!debug) {
    var options = {
      uri: url,
      qs: _.pick(task, processFields)
    };
    console.log('Processing %s, connecting to url %s', task.id, url);
    rp(options)
      .then(function (resp) {
        // TODO: Process response...
        console.log('Response to %s: ', task.id, resp);
        cb(null, resp);
      })
      .catch(function (err) {
        // Processing failed...
        console.log('Processing %s FAILED: ', task.id, err);
        cb(err, null);
      });
  } else {
    console.log('Processing %s - debug mode', task.id);
  }
}, {
     stateFile: './data/process-queue.state.json',
     store: store,
     priority: function (task, cb) {
      cb(null, task.priority || 1);
     }
   }
);

module.exports = function() {
  const app = this;

  app.process_queue = queue;

  // custom population of scans from csv
  app.get('/queues/process/add', (req, res, next) => {
    // Add scan to process queue
    //var logger = app.logger.getContext('', { path: '/queues/process/add'});
    var opts = { id: req.query.scanId, timestamp: Date.now() };
    _.defaults(opts, _.pick(req.query, processBasicFields));
    if (req.query.priority != undefined) {
      var priority = req.query.priority;
      if (priority == 'max') {
        var p = queue.maxPriority();
        priority = p? p+1 : undefined;
      } else if (priority == 'min') {
        var p = queue.minPriority();
        priority = p? p-1 : undefined;
      }
      opts.priority = priority;
    }
    console.log(opts);
    var ticket = queue.push(opts);
    res.json({ status: 'ok', size: queue.size(), ticket: ticket });
  });

  app.get('/queues/process/remove', (req, res, next) => {
    // Remove scan from process queue
    //var logger = app.logger.getContext('', { path: '/queues/process/remove'});
    queue.cancel(req.query.scanId);
    res.json({ status: 'ok', size: queue.size() });
  });

  app.get('/queues/process/list', (req, res, next) => {
    // List scans in process queue
    //var logger = app.logger.getContext('', { path: '/queues/process/list'});
    var queued = queue.list();
    res.json({ status: 'ok', queue: queued });
  });

  app.get('/queues/process/clear', (req, res, next) => {
    // List scans in process queue
    //var logger = app.logger.getContext('', { path: '/queues/process/clear'});
    queue.clear();
    res.json({ status: 'ok', size: queue.size() });
  });

  app.get('/queues/process/status', (req, res, next) => {
    // Returns basic queue status
    res.json({ status: 'ok', status: queue.status() });
  });

  app.get('/queues/process/stats', (req, res, next) => {
    // Returns basic queue stats
    res.json({ status: 'ok', stats: queue.getStats(), size: queue.size(), isPaused: queue.isPaused() });
  });

  app.get('/queues/process/pause', (req, res, next) => {
    // Pauses process queue
    queue.pause();
    queue.saveState();
    res.json({ status: 'ok', size: queue.size(), isPaused: queue.isPaused() });
  });

  app.get('/queues/process/resume', (req, res, next) => {
    // Resumes process queue
    queue.resume();
    queue.saveState();
    res.json({ status: 'ok', size: queue.size(), isPaused: queue.isPaused() });
  });

};
