function createImage(imgUrl, linkUrl) {
  var img = $('<img/>').attr('class', 'lazy').attr('data-src', imgUrl)
    .css('max-width', '100px');
  if (linkUrl) {
    var elem = $('<a></a>').attr('href', linkUrl).attr('target', '_blank')
      .append(img);
    return elem;
  } else {
    return img;
  }
}

function createActionButtons(rec) {
  var div = $('<div></div>');
  div.append(createButton('Annotate', settings.segmentAnnotatorUrl
      + '?condition=nyu-manual&userId=[username]&taskMode=fixup&modelId=nyuv2.'
      + rec.sceneName,
      'Annotate scan')
    .attr('data-check-auth', 'true')
    .attr('target', '_blank'));
  div.append(createButton('Annotations', settings.segmentAnnotationViewerUrl + '?modelId=nyuv2.' + rec.sceneName, 'View raw annotations')
    .attr('target', '_blank'));
  return div;
}

function getImageUrl(root, rec, type) {
  //513__bedroom_0132__frame27.annNYU.png
  var url = root + 'rendered-annotations/' + (rec.nyuAnnotatedIdx-1) + '__'
    + rec.sceneName + '__frame' + rec.frameIdx + (type? '.' + type  : '') + '.png';
  return url;
}

function initResultTable(params) {
  params = params || {};
  var auth = params.auth;
  var nyuAnnRoot = '/data/nyuv2/';

  var lazyImgLoadCallback = $.fn.dataTable.getLazyImgLoadCallback();
  var resultTable = $("#resultTable");
  var ajaxUrl = nyuAnnRoot + 'nyuFrameAnnotation.csv';
  resultTable.dataTable({
    "lengthMenu": [[25, 50, 100, -1], [25, 50, 100, "All"]],
    "order": [],
    "deferRender": true,
    "stateSave": true,
    "ajax": {
      url: ajaxUrl,
      dataType: "text",
      dataSrc: function(data) {
        var results = Papa.parse(data, {header: true, dynamicTyping: true, skipEmptyLines: true});
        return results.data;
      },
      complete: function(jqXhr) {
        var lastModified = jqXhr.getResponseHeader('last-modified');
        $('#lastModified').text('Modified: ' + lastModified);
      }
    },
    "dom": 'BlfripFtip',
    "buttons": [
      'csv', 'colvis', 'orderNeutral', 'selectAll', 'selectNone'
    ],
    "select": {
        style:    'multi'
    },
    "columns": [
        { "data": "nyuAnnotatedIdx" },
        { "data": "sceneName",
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var url = '/scans/browse?modelId=nyuv2.' + full.sceneName;
            var imgUrl = '/data/nyuv2/' + full.sceneName +
              '/' + full.sceneName + '_vh_clean_2_thumb.png';
            return data + '<br/>' + getHtml(createImage(imgUrl, url));
          }
        },
        { "data": "frameIdx" },
        { "data": "frameReconCoverage",
          "searchable":     false,
          "aggregation":    "avg",
          "defaultContent": 0 },
        { "data": "frameLabelCoverage",
          "searchable":     false,
          "aggregation":    "avg",
          "defaultContent": 0 },
        { // Preview image
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var imgUrl = getImageUrl(nyuAnnRoot, full);
            return getHtml(createImage(imgUrl, imgUrl));
          }
        },
        { // Preview image
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var imgUrl = getImageUrl(nyuAnnRoot, full, 'annNYU');
            return getHtml(createImage(imgUrl, imgUrl));
          }
        },
        { // Preview image
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var imgUrl = getImageUrl(nyuAnnRoot, full, 'ann');
            return getHtml(createImage(imgUrl, imgUrl));
          }
        },
        { // Action buttons
          "orderable":      false,
          "searchable":     false,
          "data":           null,
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            var actionButtons = createActionButtons(full);
            return getHtml(actionButtons);
          }
        }
    ],
    "rowCallback": function( row, aData, iDisplayIndex ) {
      //console.log(row, aData, iDisplayIndex);
      lazyImgLoadCallback(row, aData, iDisplayIndex);
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

