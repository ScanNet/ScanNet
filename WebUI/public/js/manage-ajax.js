// Simple version of _.omit
function omit(obj, fields) {
  var result = {};
  for (var x in obj) {
    if (obj.hasOwnProperty(x)) {
      var fi = fields.indexOf(x);
      if (fi < 0) {
        result[x] = obj[x];
      }
    }
  }
  return result;
}

function createFilterLink(data, field) {
  return createFilterLinks('/scans/manage', data, field);
}

function createActionButtons(scan) {
  var div = $('<div></div>');
  // Annotate buttons
  if (scan.group === 'checked' || scan.group === 'nyuv2') {
    var condition = (scan.group === 'nyuv2')? 'nyu-manual' : 'scannet-checked-manual';
    div.append(createButton('Annotate', settings.segmentAnnotatorUrl +
      '?condition=' + condition + '&userId=[username]&taskMode=fixup&modelId=' + scan.modelId, 'Annotate scan')
      .attr('data-check-auth', 'true')
      .attr('target', '_blank'));
    div.append(createButton('Annotations', settings.segmentAnnotationViewerUrl + '?modelId=' + scan.modelId, 'View raw annotations')
      .attr('target', '_blank'));
  }

  if (scan.queueState && scan.queueState.queued) {
    div.append(createButton('Cancel', null, 'Remove from process queue')
      .attr('class', scan.queueState.running? 'btn btn-danger cancelBtn' : 'btn btn-warning cancelBtn')
      .attr('data-id', scan.id)
      .attr('data-path', scan.path));
  } else {
    if (scan.group === 'staging' && scan.path === scan.id) {
      div.append(createButton('Process', null, 'Adds to process queue')
        .attr('class', 'btn btn-primary processBtn')
        .attr('data-id', scan.id)
        .attr('data-path', scan.path));
    }
  }
  if (processView) {
    div.append(createDropDown('Move', [
      createButton('to Top', '#', 'Move to top').attr('data-id', scan.id)
        .attr('class', 'moveTopBtn'),
      createButton('to End', '#', 'Move to end').attr('data-id', scan.id)
        .attr('class', 'moveBottomBtn')
    ]));
  }

  // Details and download buttons
  div.append(createButton('Details', '/api/scans/' + scan.id, 'View details'));
  if (scan.group === 'staging' || scan.group === 'checked') {
    div.append(createButton('Files', settings.scanFilesPath + '/'
      + scan.group + '/' + scan.path, 'List files')
      .attr('target', '_blank'));
    div.append(createDropDown('Download', [
      createButton('Raw', '#', 'Download scan files')
        .attr('data-id', scan.id)
        .attr('data-path', scan.group + '/' + scan.path)
        .attr('class', 'downloadRawBtn'),
      createButton('All', '#', 'Download all files')
        .attr('data-id', scan.id)
        .attr('data-path', scan.group + '/' + scan.path)
        .attr('class', 'downloadAllBtn')
    ]));
  }

  return div;
}

function createPreviewImage(scan, imgurl) {
  var url = '/scans/browse?modelId=' + scan.modelId;
  var title = JSON.stringify(omit(scan, ['files', 'stages']), undefined, 2);
  var elem = $('<a></a>').attr('href', url).attr('target', '_blank')
    .append($('<img/>').attr('class', 'lazy').attr('data-src', imgurl)
      .attr('title', title).attr('alt', scan.id).css('max-width', '128px'));
  return elem;
}


function createVideoElement(videoUrl) {
  var video = $('<video autoplay controls></video>').attr('src', videoUrl);
  return video;
}

function createVideoPreview(scan, imgurl) {
  var url = scan.videoMp4Url;
  var img = $('<img/>').attr('class', 'lazy').attr('data-src', imgurl)
    .attr('alt', scan.id).css('max-width', '128px');
  if (url) {
    img.addClass('videoPreview');
    img.attr('data-video', url);
    return img;
  } else {
    return img;
  }
}

function createProgressStatusBars(scan) {
  var stages = scan.stages;
  if (scan.lastOkStage && stages && stages.length) {
    var div = $('<div/>').attr('class', 'progressBars');
    for (var i = 0; i < stages.length; i++) {
      var stage = stages[i];
      var progressClass = (stage.ok == undefined)? 'progress-none' :
        ((stage.ok)? 'progress-ok' : 'progress-failed');
      div.append($('<span/>').attr('class', 'progress-strip ' + progressClass).attr('title', stage.name));
    }
    return div;
  }
}

function initResultTable(params) {
  params = params || {};
  var serverSide = params.serverSide;
  var lazyImgLoadCallback = $.fn.dataTable.getLazyImgLoadCallback();
  var resultTable = $("#resultTable");
  var ajaxUrl = params.ajax || '/scans/list';
  var data = params.data;
  var auth = params.auth;

  if (params.allowEdit) {
    var tagValues = getValues(data, 'tags');
    var ignoreSceneTypes = ['Please Select A Scene Type'];
    // TODO: Move to a separate file
    // 'Apartment', 'Closet', 'Hallway', 'Lobby' not in ipad app
    var basicSceneTypes =
      ['Apartment', 'Bathroom', 'Bedroom / Hotel', 'Bookstore / Library',
       'Classroom', 'Closet', 'Conference Room', 'Dining Room', 'Hallway',
       'Kitchen', 'Living room / Lounge', 'Lobby', 'Office', 'Misc.'];
    var sceneTypes = getValues(data, 'sceneType', basicSceneTypes)
      .filter(function(x) { return x && ignoreSceneTypes.indexOf(x) < 0; });
    var editor = new $.fn.dataTable.Editor( {
        ajax: {
          url: "/scans/edit",
          data: function(d) { return JSON.stringify(d); },
          contentType: "application/json; charset=utf-8"
        },
        table: "#resultTable",
        idSrc:  'id',
        fields:
        [ {
            label: "User",
            name: "userName"
          },
          {
            label: "Scene Type",
            name: "sceneType",
            type: "autoComplete",
            opts: {
              'minLength': 0,
              'source': sceneTypes
            }
          },
          {
            label: "Scene Name",
            name: "sceneName"
          },
          {
            label: "Tags",
            name: "tags",
            type: "selectize",
            options: tagValues.map(function(x) { return { label: x, value: x }; }),
            opts: {
                //plugins: ['remove_button'],
                maxItems: null,
                create: true
            }
          }
        ]
    } );

    // Activate an inline edit on click of a table cell
    resultTable.on( 'click', 'tbody td.bubble', function (e) {
        editor.bubble( this );
    } );
    resultTable.on( 'click', 'tbody td.inline', function (e) {
        editor.inline( this );
    } );
    resultTable.on( 'click', 'tbody td.inline-selectize', function (e) {
        editor.inline( this, {
          onReturn: 'none',
          buttons: { label: '&gt;', fn: function () { this.submit(); } }
        });
    } );
  }
  resultTable.dataTable({
    "lengthMenu": [[25, 50, 100, -1], [25, 50, 100, "All"]],
    "order": [],
    "deferRender": true,
    "processing": serverSide,
    "serverSide": serverSide,
    "stateSave": true,
    "data": data,
    "ajax": data? undefined : (serverSide?
      $.fn.dataTable.pipeline( {
          url: ajaxUrl,
          pages: 5 // number of pages to cache
      } ) : ajaxUrl),
    "dom": 'BlfripFtip',
    "buttons": [
      'csv', 'colvis', 'orderNeutral',// 'selectAll', 'selectNone'
    ],
    // "select": {
    //     style:    'multi'
    // },
    "columns": [
        { // Preview image of video
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            if (full.videoThumbnailUrl) {
              return getHtml(createVideoPreview(full, full.videoThumbnailUrl));
            }
          }
        },
        { // Preview image
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            if (full.previewUrl) {
              return getHtml(createPreviewImage(full, full.previewUrl));
            }
          }
        },
        { "data": "id",
          "visible": isNYU
        },
        { "data": "createdAt",
          "visible": !isNYU,
          "defaultContent": "" },
        { "data": "updatedAt",
          "defaultContent": "" },
        { "data": "numColorFrames",
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "scanSecs",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "totalProcessingSecs",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": "",
          render: function ( data, type, full, meta ) {
            return full.totalProcessingTime;
          }
        },
        { "data": "objects",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "labels",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "floorArea",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "floorAreaFilled",
          "visible": false,
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "floorAreaRatio",
          "visible": false,
          "searchable":     false,
          "aggregation":    "average",
          "defaultContent": ""
        },
        { "data": "deviceName",
          "visible": !isNYU,
          "defaultContent": "",
          render: function ( data, type, full, meta ) {
            return getHtml(createFilterLink(full, "deviceName"));
          }
        },
        { "data": "userName",
          "visible": !isNYU,
          "className": "inline",
          "defaultContent": "" },
        { "data": "sceneName",
          "visible": !isNYU,
          "className": "inline",
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            if (full.sceneLabel !== full.sceneName) {
              var span = $('<div/>').append(
                $('<small class="text-muted"/>').text(full.sceneLabel));
              var div = $('<div/>');
              div.append(data).append(span);
              return getHtml(div);
            } else {
              return data;
            }
          }
        },
        { "data": "sceneType",
          "className": "inline",
          "defaultContent": "" },
        { "data": "group",
          "visible": !isNYU,
          "defaultContent": "",
          render: function ( data, type, full, meta ) {
            return getHtml(createFilterLink(full, "group"));
          }
        },
        { "data": "subgroup",
          "visible": false,
          "defaultContent": "" },
        { "data": "lastOkStage",
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var progressDiv = createProgressStatusBars(full);
            if (progressDiv) {
              var div = $('<div/>');
              div.append(createFilterLink(full, "lastOkStage")).append(progressDiv);
              return getHtml(div);
            } else {
              return data? getHtml(createFilterLink(full, "lastOkStage")) : data;
            }
          }
        },
        { "data": "tags",
          "type": "array",
          "className": "inline-selectize",
          "defaultContent": "" },
        { "data": "totalVertices",
          "searchable":     false,
          "aggregation":    "sum",
          "defaultContent": ""
        },
        { "data": "percentComplete",
          "searchable":     false,
          "aggregation":    "average",
          "defaultContent": ""
        },
        { // Action buttons
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            return getHtml(createActionButtons(full));
          }
        }
    ],
    "rowCallback": function( row, aData, iDisplayIndex ) {
      //console.log(row, aData, iDisplayIndex);
      lazyImgLoadCallback(row, aData, iDisplayIndex);
      var imgs = $(row).find('img.videoPreview');
      imgs.each(function( index ) {
        var img = $(this);
        if (!img.attr('data-video-bound')) {
          img.click(function(event) {
            var videoUrl = img.attr('data-video');
            video = createVideoElement(videoUrl);
            video.click(function(event) {
              video.hide();
              img.show();
            });
            img.parent().append(video);
            img.hide();
          });
          img.attr('data-video-bound', true);
        }
      });
      processQueue.initProcessButtons(row);
      if (auth) {
        auth.addCheck($(row));
      }
      return row;
    },
    "initComplete": function() {
      $.fn.dataTable.addColumnFilters({ table: resultTable });
      $.fn.dataTable.addColumnAggregations({ table: resultTable });
      resultTable.css('visibility', 'visible');
      $('#loadingMessage').hide();
    }
  });
}

