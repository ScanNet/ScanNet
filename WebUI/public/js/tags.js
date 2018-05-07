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
        { "data": "name" },
        { "data": "scans" },
        { // Action buttons
          "orderable":      false,
          "searchable":     false
        }
    ],
    "rowCallback": function( row, aData, iDisplayIndex ) {
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
