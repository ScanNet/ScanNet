function initResultTable(params) {
  var lazyImgLoadCallback = $.fn.dataTable.getLazyImgLoadCallback();
  var resultTable = $("#resultTable");
  resultTable.dataTable({
    "lengthMenu": [[25, 50, 100, -1], [25, 50, 100, "All"]],
    "order": [],
    "deferRender": false,
    "stateSave": true,
    "dom": 'BlfripFtip',
    "buttons": [
      'csv', 'colvis', 'orderNeutral', 'selectAll', 'selectNone'
    ],
    "select": {
        style:    'multi'
    },
    "columns": [
        { // Preview image
          "orderable":      false,
          "searchable":     false
        },
        { "data": "id" },
        { "data": "createdAt" },
        { "data": "updatedAt" },
        { "data": "deviceName" },
        { "data": "sceneLabel" },
        { "data": "sceneType" },
        { "data": "group" },
        { "data": "lastOkStage",
          "defaultContent": ""
        },
        { "data": "tags",
          "defaultContent": ""
        },
        { // Action buttons
          "orderable":      false,
          "searchable":     false
        }
    ],
    "rowCallback": function( row, aData, iDisplayIndex ) {
      //console.log(row, aData, iDisplayIndex);
      lazyImgLoadCallback(row, aData, iDisplayIndex);
      processQueue.initProcessButtons(row);
      return row;
    },
    "initComplete": function() {
      $.fn.dataTable.addColumnFilters({ table: resultTable });
      resultTable.css('visibility', 'visible');
      $('#loadingMessage').hide();
    }
  });
}
