function createActionButtons(device) {
  var div = $('<div></div>');
  div.append(createButton('Scans', '/scans/manage?deviceName=' + device.name, 'View scans'));
  return div;
}

function initResultTable(params) {
  params = params || {};
  var data = params.data;

  var lazyImgLoadCallback = $.fn.dataTable.getLazyImgLoadCallback();
  var resultTable = $("#resultTable");
  resultTable.dataTable({
    "lengthMenu": [[25, 50, 100, -1], [25, 50, 100, "All"]],
    "order": [],
    "deferRender": false,
    "stateSave": true,
    "data": data,
    "dom": 'BlfripFtip',
    "buttons": [
      'csv', 'colvis', 'orderNeutral', 'selectAll', 'selectNone'
    ],
    "select": {
        style:    'multi'
    },
    "columns": [
        { "data": "ids",
          "type": "array",
          "defaultContent": "",
          "render": function ( data, type, full, meta ) {
            return getHtml(createFilterLinks('/scans/manage', full, 'ids', { queryField: 'deviceId' }));
          }
        },
        { "data": "name",
          "defaultContent": ""
        },
        { "data": "scans.length",
          "defaultContent": "" },
        { // Action buttons
          "data": null,
          "orderable":      false,
          "searchable":     false,
          "render": function ( data, type, full, meta ) {
            return getHtml(createActionButtons(full));
          }
        }
    ],
    "rowCallback": function( row, aData, iDisplayIndex ) {
      //console.log(row, aData, iDisplayIndex);
      //console.log(row, aData, iDisplayIndex);
      lazyImgLoadCallback(row, aData, iDisplayIndex);
      return row;
    },
    "initComplete": function() {
      $.fn.dataTable.addColumnFilters({ table: resultTable });
      resultTable.css('visibility', 'visible');
      $('#loadingMessage').hide();
    }
  });
}
