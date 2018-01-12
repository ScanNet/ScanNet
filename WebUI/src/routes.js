// Using the simplest possible proxying for proxying data located on servers
const csv = require('csv');
const errors = require('feathers-errors');
const _ = require('lodash');

module.exports = function() {
  const app = this;
  const scanService = app.service('/api/scans');
  const scanmetaService = app.service('/api/scanmeta');

  function groupByArrayField(data, field) {
    groupedByField = {};
    for (var i = 0; i < data.length; i++) {
      var d = data[i];
      var values = d[field];
      if (values) {
        for (var j = 0; j < values.length; j++) {
          var v = values[j];
          if (v != undefined) {
            if (groupedByField[v]) {
              groupedByField[v].push(d);
            } else {
              groupedByField[v] = [d];
            }
          }
        }
      }
    }
    return groupedByField;
  }

  // render home page
  //app.get('/', (req, res) => res.render('index'));

  // render annotations
  app.get('/scans/annotations', (req, res, next) => {
    res.render('annotations', {
      settings: app.get('settings')
    });
  });

  // render scans by devices
  app.get('/scans/devices/id', (req, res, next) => {
    scanService.find({query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByDeviceId = _.groupBy(scans.filter( (x) => {return x.deviceId;}), 'deviceId');
      } else {
        scansByDeviceId = {};
      }
      var devices = _.map(scansByDeviceId, (deviceScans, deviceId) => {
        var deviceNames = _.map(deviceScans, (scan) => { return scan.deviceName; });
        deviceNames = _.uniq(_.filter(deviceNames, (name) => { return name; }));
        return { id: deviceId, names: deviceNames, scans: deviceScans };
      });
      res.render('devicesById', {
        devices: devices,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  app.get('/scans/devices/name', (req, res, next) => {
    scanService.find({query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByDeviceName = _.groupBy(scans.filter( (x) => {return x.deviceName;}), 'deviceName');
      } else {
        scansByDeviceName = {};
      }
      var devices = _.map(scansByDeviceName, (deviceScans, deviceName) => {
        var deviceIds = _.map(deviceScans, (scan) => { return scan.deviceId; });
        deviceIds = _.uniq(_.filter(deviceIds, (id) => { return id; }));
        return { name: deviceName, ids: deviceIds, scans: deviceScans };
      });
      res.render('devicesByName', {
        devices: devices,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  // render scans by sceneName
  app.get('/scans/scenes', (req, res, next) => {
    //res.render('manage');
    scanService.find({ sceneName: { '$ne': null }, query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByScene = _.groupBy(scans.filter( (x) => {return x.sceneName;}), 'sceneName');
      } else {
        scansByScene = {};
      }
      var scenes = _.map(scansByScene, (groupedScans, name) => {
        var sceneTypes = _.map(groupedScans, (scan) => { return scan.sceneType; });
        sceneTypes = _.uniq(_.filter(sceneTypes, (sceneType) => { return sceneType; }));
        return { name: name, type: sceneTypes, scans: groupedScans };
      });
      res.render('scenes', {
        scenes: scenes,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  // render scans by sceneType
  app.get('/scans/scenes/type', (req, res, next) => {
    if (!req.query['group']) {
      req.query['group'] = { '$ne': 'nyuv2' };
    }
    scanService.find({query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByScene = _.groupBy(scans.filter( (x) => {return x.sceneType;}), 'sceneType');
      } else {
        scansByScene = {};
      }
      var scenes = _.map(scansByScene, (groupedScans, type) => {
        var byName = _.groupBy(groupedScans.filter( (x) => {return x.sceneName;}), 'sceneName');
        return { type: type, scans: groupedScans, numScenes: _.size(byName) };
      });
      res.render('scenesByType', {
        scenes: scenes,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  // render scans by user
  app.get('/scans/users', (req, res, next) => {
    scanService.find({ userName: { '$ne': null }, query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByUser = _.groupBy(scans.filter( (x) => {return x.userName;}), 'userName');
      } else {
        scansByUser = {};
      }
      var users = _.map(scansByUser, (groupedScans, name) => {
        return { name: name, scans: groupedScans };
      });
      res.render('users', {
        users: users,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  // render scans by tag
  app.get('/scans/tags', (req, res, next) => {
    scanService.find({ tag: { '$ne': null }, query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        scansByTag = groupByArrayField(scans, 'tags');
      } else {
        scansByTag = {};
      }
      var tags = _.map(scansByTag, (groupedScans, name) => {
        return { name: name, scans: groupedScans };
      });
      res.render('tags', {
        tags: tags,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  // render our list of messages retrieving them from our service
  app.get('/scans/browse', (req, res, next) => {
    res.render('browse');
  });

  app.get('/scans/list', (req, res, next) => {
    var queued = app.process_queue.list();
    var queuedById = _.keyBy(queued, 'id');
    scanService.find({query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        for (var i = 0; i < scans.length; i++) {
          var scan = scans[i];
          var queueState = queuedById[scan['id']];
          if (queueState) {
            scan.queueState = { queued: true, running: !!queueState.lock };
          }
        }
      }
      // Hacky way to get count of total total
      result.data = scans;
      scanService.find({query: {'$limit': 1}}).then( all => {
        result.totalAll = all.total;
        res.json(result);
      }).catch(next);
    }).catch(next);
  });

  app.get('/scans/browse/nyu', (req, res, next) => {
    req.query['group'] = 'nyuv2';
    scanService.find({query: req.query}).then(result => {
      var scans = result.data ? result.data : result;
      res.render('manage', { scans: scans, total: result.total,
        settings: app.get('settings'),
        nyu: true
      });
    }).catch(next);
  });

  app.get('/scans/browse/nyu/frames', (req, res, next) => {
    res.render('nyuframes', {
      settings: app.get('settings')
    });
  });

  app.get('/scans/manage', (req, res, next) => {
    //res.render('manage');
    if (!req.query['$sort']) {
      req.query['$sort'] = { 'createdAt': -1 };
    }
    if (!req.query['group']) {
      req.query['group'] = { '$ne': 'nyuv2' };
    }
    //console.log(req.query);
    var queued = app.process_queue.list();
    var queuedById = _.keyBy(queued, 'id');
    scanService.find({query: req.query}).then(result => {
      // This handles paginated and non-paginated services
      var scans = result.data ? result.data : result;
      if (scans && scans.length) {
        for (var i = 0; i < scans.length; i++) {
          var scan = scans[i];
          var queueState = queuedById[scan['id']];
          if (queueState) {
            scan.queueState = { queued: true, running: !!queueState.lock };
          }
        }
      }
      res.render('manage', { scans: scans, total: result.total,
        processQueue: app.process_queue.status(),
        manageView: true,
        settings: app.get('settings')
      });
    }).catch(next);
  });

  app.get('/scans/process/:scanId', (req, res, next) => {
    app.process_queue.push( { id: req.params.scanId });
    res.json({ status: 'ok', size: app.process_queue.size() });
  });

  app.get('/scans/process', (req, res, next) => {
    var queued = app.process_queue.list();
    scanService.find({}).then(result => {
      // TODO: Filter by the ones that are on the process queue
      var scans = result.data ? result.data : result;
      var scansById = _.keyBy(scans, 'id');
      var queuedScans = queued.map( queueState => {
        var scanId = queueState.id;
        var scan = scansById[scanId];
        if (!scan) {
          scan = {
            id: scanId
          };
        }
        scan.queueState = { queued: true, running: !!queueState.lock };
        return scan;
      });
      res.render('manage', { scans: queuedScans,
        total: queued.length,
        processQueue: app.process_queue.status(),
        processView: true,
        settings: app.get('settings')
      });
    }).catch(next);
  });


  app.post('/scans/edit', (req, res, next) => {
    var logger = app.logger.getContext('', { path: '/scans/edit'});
    var params = req.body;
    logger.info('Processing ' + JSON.stringify(params));
    if (params.action === 'edit') {
      if (!params.data) {
        throw new errors.GeneralError(
          { message: 'No data for /scans/edit: ' + params.action });
      }
      // Editing
      //var patchData = _.map(params.data, function(v,k) { v._id = k; return v; });
      //console.log(patchData);
      // TODO: Handle multple rows being editted
      _.each(params.data, function(v,k) {
        scanmetaService.patch(k, v, { nedb: { upsert: true } }).then(result => {
          scanService.patch(k, v).then(scanResult => {
            if (!_.isArray(scanResult)) {
              scanResult = [scanResult];
            }
            _.each(result, function(f) {
              f.id = f._id;
              delete f.files;
              delete f._id;
            });
            res.json({ status: 'ok', data: scanResult });
          }).catch(next);
        }).catch(next);
      });
    } else {
      throw new errors.GeneralError(
          { message: 'Unsupported action for /scans/edit: ' + params.action });
    }
  });


  // TODO: Move logic into service
  // custom population of scans from csv
  app.post('/scans/populate', (req, res, next) => {
    var logger = app.logger.getContext('', { path: '/scans/populate'});
    var preserveKeys = ['sceneName', 'sceneType'];
    function error(e) {
      console.log(e);
      logger.error(e);
      res.json(new errors.GeneralError(
        { message: 'Error processing /scans/populate', error: e }));
    }

    function populate(data) {
      logger.info('Populating ' + data.length + ' scans');
      scanService.create(data).then( result => {
        scanService.find().then( found => {
          //console.log(found);
          res.json({ status: 'ok', processed: result.length, total: found.total });
        }).catch(error);
      }).catch(error);
    }

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

    function normalizeObject(obj) {
      for (k in obj) {
        if (obj.hasOwnProperty(k)) {
          obj[k] = normalizeValue(obj[k]);
        }
      }
    }

    function populateAll(err, data) {
      //console.log(data);
      if (data) {
        if (!_.isArray(data)) {
          data = _.values(data);
        }
        logger.info('Processing /scans/populate for ' + data.length + ' scans');
        for (var i = 0; i < data.length; i++) {
           normalizeObject[data[i]];
           if (data[i].path) {
              var subgroup = data[i].path.replace(data[i].id, '');
              subgroup = _.trim(subgroup, '/\\');
              if (subgroup) {
                data[i].subgroup = subgroup;
              }
           }
           data[i]._orig = _.pick(data, preserveKeys);
           data[i]._id = data[i].id;
        }
        if (req.query.group) {
          for (var i = 0; i < data.length; i++) {
            data[i].group = req.query.group;
          }
        }

        function replaceAndPopulate() {
          if (req.query.replace) {
            // Replace all
            if (req.query.replace == 'group' && req.query.group) {
              logger.info('Remove group ' + req.query.group);
              scanService.remove(null, {query: {group: req.query.group}} ).then( result => {
                populate(data);
              }).catch(error);
            } else if (req.query.replace == 'all') {
              logger.info('Remove all');
              scanService.remove(null, {}).then( result => {
                populate(data);
              }).catch(error);
            } else {
              logger.info('Remove individual scans');
              var ids = data.map( x => x.id );
              scanService.remove(null, {query: {id: { $in: ids }} }).then( result => {
                populate(data);
              }).catch(error);
            }
          } else {
            populate(data);
          }
        }

        scanmetaService.find({}).then( metaResults => {
          // combine data and metaResults
          if (metaResults.data) {
            var metaById = _.keyBy(metaResults.data, '_id');
            for (var i = 0; i < data.length; i++) {
              var meta = metaById[data[i].id];
              if (meta) {
                _.assign(data[i], meta);
              }
            }
          }

          replaceAndPopulate();
        }).catch(error);
      } else {
        if (err) {
          logger.error(err);
        } else {
          logger.error('No data');
        }
        res.json(new errors.GeneralError(
          { message: 'Error processing /scans/populate: ' + err }));
      }
    }

    if (typeof req.body === 'string') {
      // Convert from csv to json
      csv.parse(req.body, { skip_empty_lines: true, columns: true }, populateAll);
    } else {
      populateAll(null, req.body);
    }
  });

};
