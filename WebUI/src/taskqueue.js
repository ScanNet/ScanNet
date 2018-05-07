const fs = require('fs');
var BasicQueue = require('better-queue');
var MemoryStore = require('better-queue-memory');
var SqlStore = require('better-queue-sql');
var _ = require('lodash');

// Improvements to SqlStore
SqlStore.prototype.getTaskStates = function(n, fields, cb) {
  var self = this;
  var first = true;
  var q = self.adapter.knex(self.tableName).orderBy('lock', 'desc').orderBy('priority', 'DESC').orderBy('added', first ? 'ASC' : 'DESC');
  q = (fields)? q.select(fields) : q.select();
  q = (n > 0)? q.limit(n) : q;
  q.then(function(rows) {
    cb(null, rows);
  }).catch(function(error) {
    cb(error, null);
  });
};

// Improvements to MemoryStore
MemoryStore.prototype.getTaskStates = function(n, fields, cb) {
  var self = this;
  var running = _.flatMap( self._running, (tasks, lockId) => {
    return _.map( tasks, (task, taskId) => {
      return {
        id: taskId,
        task: task,
        lock: lockId,
        priority: self._priorities[taskId]
      };
    });
  });
  var queued = this._queue.map( taskId => {
    return {
      id: taskId,
      task: self._tasks[taskId],
      priority: self._priorities[taskId]
    };
  });

  var all = _.concat([], running, queued);
  all = (n > 0)? _.take(all, n) : all;
  if (cb) {
    cb(null, all);
  }
  return all;
};


// MemoryStored that is backed by a sqlstore
// Always pushes changes to backend store
// But keeps a cache of queue tasks
function CachedStore(store) {
  this._store = store;
  this._taskIds = [];     // Array of taskIds
  this._taskStates = [];  // Array of task states
  this._taskStatesById = {};  // TaskId to task state
}

CachedStore.prototype.connect = function (cb) {
  var self = this;
  this._store.connect( self.__wrapWithUpdateQueue(cb) );
};

CachedStore.prototype.__wrapWithUpdateQueue = function(cb) {
  var self = this;
  return function(err, res) {
    if (!err) {
      self._store.getTaskStates(0, null, function(err, rows) {
        self._taskStates = rows || [];
        self._taskIds = _.map(self._taskStates, 'id');
        self._taskStatesById = _.keyBy(self._taskStates, 'id');
        console.log('In queue: ' + self._taskStates.length);
        cb(null, res);
      });
    } else {
      cb(err, res);
    }
  };
};

CachedStore.prototype.getTask = function (taskId, cb) {
  return this._store.getTask(taskId, cb);
};

CachedStore.prototype.deleteTask = function (taskId, cb) {
  return this._store.deleteTask(taskId, this.__wrapWithUpdateQueue(cb));
};

CachedStore.prototype.putTask = function (taskId, task, priority, cb) {
  return this._store.putTask(taskId, task, priority, this.__wrapWithUpdateQueue(cb));
};

CachedStore.prototype.takeFirstN = function (n, cb) {
  return this._store.takeFirstN(n, this.__wrapWithUpdateQueue(cb));
};

CachedStore.prototype.takeLastN = function (n, cb) {
  return this._store.takeLastN(n, this.__wrapWithUpdateQueue(cb));
};

CachedStore.prototype.getLock = function (lockId, cb) {
  return this._store.getLock(lockId, cb);
};

CachedStore.prototype.getRunningTasks = function (cb) {
  return this._store.getRunningTasks(cb);
};

CachedStore.prototype.releaseLock = function (lockId, cb) {
  return this._store.releaseLock(lockId, this.__wrapWithUpdateQueue(cb));
};

function Queue(process, opts) {
  opts = opts || {};
  BasicQueue.call(this, process, opts);
  this._stateFile = opts.stateFile;
  this.loadState();
}

Queue.prototype = Object.create(BasicQueue.prototype);
Queue.prototype.constructor = Queue;

// Improvements to the queue
Queue.prototype.size = function() {
  var taskIds = this.taskIds();
  if (this.length !== taskIds.length) {
    console.log('Got taskIds length=%d, cached length=%d', taskIds.length, this.length);
  }
  return taskIds.length;
};

Queue.prototype.taskIds = function() {
  var tasks = this.list();
  if (tasks) {
    return _.map(tasks, 'id');
  }
};

Queue.prototype.list = function() {
  var store = this._store;
  if (store._taskStates) {
    return store._taskStates;
  } else if (store._queue && store._tasks) {
    var tasks = store.getTaskStates();
    return tasks;
  }
};

Queue.prototype.firstTask = function() {
  var tasks = this.list();
  if (tasks && tasks.length > 0) {
    return tasks[0];
  }
};

Queue.prototype.lastTask = function() {
  var tasks = this.list();
  if (tasks && tasks.length > 0) {
    return tasks[tasks.length-1];
  }
};

Queue.prototype.maxPriority = function() {
  var tasks = this.list();
  return _.maxBy(tasks, 'priority').priority;
};

Queue.prototype.minPriority = function() {
  var tasks = this.list();
  return _.minBy(tasks, 'priority').priority;
};

Queue.prototype.clear = function() {
  // Clears the queue
  this.pause();
  var taskIds = this.taskIds();
  for (var i = 0; i < taskIds.length; i++) {
    console.log('cancel %s', taskIds[i]);
    this.cancel(taskIds[i]);
  }
  this.resume();
};

Queue.prototype.isPaused = function() {
  return this._stopped;
};

Queue.prototype.status = function() {
  return { isPaused: this.isPaused(), size: this.size() };
};

Queue.prototype.saveState = function() {
  if (this._stateFile) {
    var state = {
      isPaused: this.isPaused()
    };
    fs.writeFileSync(this._stateFile, JSON.stringify(state), 'utf-8');
  }

};


Queue.prototype.loadState = function() {
  if (this._stateFile && fs.existsSync(this._stateFile)) {
    var state = fs.readFileSync(this._stateFile, 'utf-8');
    if (state) {
      state = JSON.parse(state);
      if (state.isPaused) {
        this.pause();
      }
    }
  }
};

Queue.getCachedSqlStore = function(opts) {
  var store = new SqlStore(opts);
  var cached = new CachedStore(store);
  return cached;
};

module.exports = Queue;
