// Utility functions for working with datatables
// See https://datatables.net

// Utility functions for creating html for datatables
function getValues(data, field, includeValues) {
  var values = _.map(data, function(d) { return d[field]; });
  values = _.flatten(values);
  values = _.filter(values, function(x) { return (x != undefined) && x.length > 0; });
  if (includeValues) {
    values = _.concat(values, includeValues);
  }
  values = _.uniq(values); // uniquify
  values = values.sort();
  return values;
}

function createLink(element, url, target) {
  var link = $('<a></a>').attr('href', url);
  if (typeof element === 'string') {
    link.text(element);
  } else {
    link.append(element);
  }
  if (target) {
    link.attr('target', target);
  }
  return link;
}

function createFilterLinks(url, data, field, opts) {
  opts = opts || {};
  var queryField = opts.queryField || field;
  var value = data[field];
  if (_.isArray(value)) {
    var div = $('<div></div>');
    for (var i = 0; i < value.length; i++) {
      var params = {};
      params[queryField] = value[i];
      div.append(createLink(value[i], url + '?' + $.param(params)));
      if (i < value.length-1) {
        div.append(',').append($('<br/>'));
      }
    }
    return div;
  } else {
    var params = {};
    params[queryField] = value;
    var label = value;
    if (opts.indicateSpecialValues) {
      if (value == null) {
        label = $('<span class="glyphicon glyphicon-stop"></span>').append('NULL');
      } else if (value === '') {
        label = $('<span class="glyphicon glyphicon-stop"></span>').append('EMPTY');
      }
    }
    return createLink(label, url + '?' + $.param(params));
  }
}

function createButton(label, url, desc) {
  var button = $('<a></a>')
    .attr('class', 'btn btn-primary')
    .attr('role', 'button')
    .attr('href', url)
    .attr('title', desc);
  button.append(label);
  return button;
}

function createDropDown(label, buttons) {
  var btnGroup = $('<div></div').attr('class', 'btn-group');
  var mainButton = $('<button></button')
    .attr('type', 'button')
    .attr('class', 'btn btn-primary dropdown-toggle')
    .attr('data-toggle', 'dropdown')
    .text(label)
    .append('<span class="caret"></span>');
  btnGroup.append(mainButton);
  var menu = $('<ul></ul>').attr('class', 'dropdown-menu').attr('role', 'menu');
  for (var i = 0; i < buttons.length; i++) {
    menu.append($('<li></li>').attr('role', 'presentation')
      .append(buttons[i].attr('role', 'menuitem').attr('tabindex', '-1')));
  }
  btnGroup.append(menu);
  return btnGroup;
}

function getHtml(element) {
  if (typeof element === 'string') { return element; }
  else {
    return element.get(0).outerHTML;
  }
}

//
// Pipelining function for DataTables. To be used to the `ajax` option of DataTables
// See https://datatables.net/examples/server_side/pipeline.html
//
$.fn.dataTable.pipeline = function ( opts ) {
  //console.log(opts)
  // Configuration options
  var conf = $.extend( {
    pages: 5,     // number of pages to cache
    url: '',      // script url
    data: null,   // function or object with parameters to send to the server
                  // matching how `ajax.data` works in DataTables
    method: 'GET' // Ajax HTTP method
  }, opts );

  // Private variables for storing the cache
  var cacheLower = -1;
  var cacheUpper = null;
  var cacheLastRequest = null;
  var cacheLastJson = null;

  return function ( request, drawCallback, settings ) {
    var ajax          = false;
    var requestStart  = request.start;
    var drawStart     = request.start;
    var requestLength = request.length;
    var requestEnd    = requestStart + requestLength;

    if ( settings.clearCache ) {
      // API requested that the cache be cleared
      ajax = true;
      settings.clearCache = false;
    }
    else if ( cacheLower < 0 || requestStart < cacheLower || requestEnd > cacheUpper ) {
      // outside cached data - need to make a request
      ajax = true;
    }
    else if ( JSON.stringify( request.order )   !== JSON.stringify( cacheLastRequest.order ) ||
      JSON.stringify( request.columns ) !== JSON.stringify( cacheLastRequest.columns ) ||
      JSON.stringify( request.search )  !== JSON.stringify( cacheLastRequest.search )
    ) {
      // properties changed (ordering, columns, searching)
      ajax = true;
    }

    // Store the request for checking next time around
    cacheLastRequest = $.extend( true, {}, request );

    if ( ajax ) {
      // Need data from the server
      if ( requestStart < cacheLower ) {
        requestStart = requestStart - (requestLength*(conf.pages-1));

        if ( requestStart < 0 ) {
          requestStart = 0;
        }
      }

      cacheLower = requestStart;
      cacheUpper = requestStart + (requestLength * conf.pages);

      request.start = requestStart;
      request.length = requestLength*conf.pages;

      // Provide the same `data` options as DataTables.
      if ( $.isFunction ( conf.data ) ) {
        // As a function it is executed with the data object as an arg
        // for manipulation. If an object is returned, it is used as the
        // data object to submit
        var d = conf.data( request );
        if ( d ) {
          $.extend( request, d );
        }
      }
      else if ( $.isPlainObject( conf.data ) ) {
        // As an object, the data given extends the default
        $.extend( request, conf.data );
      }

      // Reinterpret parameters
      var requestData = {
        '$limit': request.length,
        '$skip': request.start
      };
      if (request.order && request.order.length) {
        var sort = {};
        for (var i = 0; i < request.order.length; i++) {
          var s = request.order[i];
          var colname = request.columns[s.column].data;
          if (colname) {
            sort[colname] = (s.dir === 'asc')? +1 : -1;
          }
        }
        requestData['$sort'] = sort;
      }
      // Not yet supported (see https://github.com/feathersjs/feathers/issues/334)
      //if (request.search && request.search.value) {
      //  requestData['$search'] = [request.search.value];
      //}
      console.log('requesting data from ' + conf.url, request, requestData);
      settings.jqXHR = $.ajax( {
        'type':     conf.method,
        'url':      conf.url,
        'data':     requestData,
        'dataType': 'json',
        'cache':    false,
        'success':  function ( json ) {
          console.log(json);
          json.recordsFiltered = json.total;
          json.recordsTotal = json.totalAll;

          cacheLastJson = $.extend(true, {}, json);

          if ( cacheLower != drawStart ) {
            json.data.splice( 0, drawStart-cacheLower );
          }
          if ( requestLength >= -1 ) {
            json.data.splice( requestLength, json.data.length );
          }

          drawCallback( json );
        }
      } );
    }
    else {
      var json = $.extend( true, {}, cacheLastJson );
      json.draw = request.draw; // Update the echo for each response
      json.data.splice( 0, requestStart-cacheLower );
      json.data.splice( requestLength, json.data.length );

      drawCallback(json);
    }
  };
};

// Define custom buttons (see https://datatables.net/extensions/buttons/custom)
$.fn.dataTable.ext.buttons.orderNeutral = {
  text: 'Default order',
  action: function ( e, dt, node, config ) {
    dt.order.neutral().draw();
  }
};

$.fn.dataTable.addFooters = function(opts) {
  var table = opts.table;
  var columns = opts.columns;
  var tfoot = table.find('tfoot');
  if (!tfoot.length) {
    tfoot = $('<tfoot></tfoot>');
    table.append(tfoot);
  }
  var tfr = tfoot.find('tr');
  if (!tfr.length) {
    tfr = $('<tr></tr>');
    tfoot.append(tfr);
  }
  for (var i = 0; i < columns.length; i++) {
    var title = columns[i].title;
    tfr.append($('<th></th>').text(title));
  }
};

$.fn.dataTable.addColumnFilters = function(opts) {
  var table = opts.table;
  //   // Setup - add a text input to each footer cell
  // table.find('tfoot th').each( function () {
  //   var title = $(this).text();
  //   $(this).html( '<input type="text" placeholder="'+title+'" />' );
  // });

  var state = table.DataTable().state();
  // Apply the search
  var columns = table.DataTable().settings().init().columns;
  table.DataTable().columns().every( function (index) {
    var searchable = columns[index].searchable !== false;
    var searchText = state? state.columns[index].search.search : '';
    if (searchable) {
      var that = this;
      var data = this.data();
      // Build up values
      var values = data
        .flatten()
        .map( function(x) { return (x != undefined)? x.toString() : null; })
        .filter( function(x) { return (x != undefined) && x.length > 0; })
        .sort()       // Sort data alphabetically
        .unique()
        .toArray();
      var title = $(this.footer()).text();
      var maxTextLength = values.length > 0?
        Math.max.apply(null, values.map( function(x) { return x.length; })) : 0;

      var input = $('<input type="text" placeholder="'+title+'" />"')
        .val(searchText)
        .attr('size', Math.min(20,maxTextLength+2))
        .appendTo( $(this.footer()).empty() )
        .on('keyup change', function () {
          if (that.search() !== this.value) {
            that.search( this.value ).draw();
          }
        })  // Trigger search
        .autocomplete({
          minLength: 0,
          source: values,
          position: {
            my: 'left bottom', at: 'left top'
          },
          select: function (event, ui) {
            var value = ui.item.value;
            if (that.search() !== value) {
              that.search( value ).draw();
            }
          }
        }); // Set up autocomplete
    }
  });
};

$.fn.dataTable.addColumnAggregations = function(opts) {
  var table = opts.table;
  //   // Setup - add a text input to each footer cell
  // table.find('tfoot th').each( function () {
  //   var title = $(this).text();
  //   $(this).html( '<input type="text" placeholder="'+title+'" />' );
  // });

  var state = table.DataTable().state();
  // Apply the search
  var columns = table.DataTable().settings().init().columns;
  table.DataTable().columns().every( function (index) {
    var aggregation = columns[index].aggregation;
    if (aggregation === 'sum' || aggregation === 'avg' || aggregation === 'average') {
      var that = this;
      var data = this.data();

      // Remove the formatting to get integer data for summation
      var numVal = function ( i ) {
        return (typeof i === 'string') ? i.replace(/[\$,]/g, '')*1 :
          (typeof i === 'number') ?
            i : 0;
      };
      // Total over all pages
      count = data.length;
      total = data
        .reduce( function (a, b) {
          return numVal(a) + numVal(b);
        }, 0 );

      // Total over this page
      // pageTotal = table.DataTable()
      //     .column( iCol, { page: 'current'} )
      //     .data()
      //     .reduce( function (a, b) {
      //         return numVal(a) + numVal(b);
      //     }, 0 );

      if (aggregation === 'avg' || aggregation === 'average') {
        aggregate = total / data.count();
      } else {
        aggregate = total;
      }

      // Update footer
      $( this.footer() ).html(
        aggregate
//          pageTotal +' ( '+ total +' total)'
      );
    }
  });
};


$.fn.dataTable.getLazyImgLoadCallback = function() {
  var loadedCount = 0;
  return function( row, aData, iDisplayIndex ) {
    //console.log(row, aData, iDisplayIndex);
    var imgs = $(row).find('img.lazy');
    imgs.each(function( index ) {
      var img = $(this);
      if (img.attr('src') !== img.attr('data-src')) {
        loadedCount += 1;
        console.log('setting src ' + img.attr('data-src') + ' for ' + loadedCount);
        img.attr('src', img.attr('data-src'));
      }
    });
  };
};


// Register an API method that will empty the pipelined data, forcing an Ajax
// fetch on the next draw (i.e. `table.clearPipeline().draw()`)
$.fn.dataTable.Api.register( 'clearPipeline()', function () {
  return this.iterator( 'table', function ( settings ) {
    settings.clearCache = true;
  } );
} );

// Order neutral from https://datatables.net/plug-ins/api/order.neutral()
// //cdn.datatables.net/plug-ins/1.10.12/api/order.neutral().js
$.fn.dataTable.Api.register( 'order.neutral()', function () {
  return this.iterator( 'table', function ( s ) {
    s.aaSorting.length = 0;
    s.aiDisplay.sort( function (a,b) {
      return a-b;
    } );
    s.aiDisplayMaster.sort( function (a,b) {
      return a-b;
    } );
  } );
} );


// Plugin for jquery ui autocomplete
// https://editor.datatables.net/plug-ins/field-type/editor.autoComplete
(function ($, DataTable) {


  if ( ! DataTable.ext.editorFields ) {
    DataTable.ext.editorFields = {};
  }

  var _fieldTypes = DataTable.Editor ?
    DataTable.Editor.fieldTypes :
    DataTable.ext.editorFields;

  _fieldTypes.autoComplete = {
    create: function ( conf ) {
      conf._input = $('<input type="text" id="'+conf.id+'">')
        .autocomplete( conf.opts || {} );

      return conf._input[0];
    },

    get: function ( conf ) {
      return conf._input.val();
    },

    set: function ( conf, val ) {
      conf._input.val( val );
    },

    enable: function ( conf ) {
      conf._input.autocomplete( 'enable' );
    },

    disable: function ( conf ) {
      conf._input.autocomplete( 'disable' );
    },

    // Non-standard Editor method - custom to this plug-in
    node: function ( conf ) {
      return conf._input;
    },

    update: function ( conf, options ) {
      conf._input.autocomplete( 'option', 'source', options );
    }
  };


})(jQuery, jQuery.fn.dataTable);
