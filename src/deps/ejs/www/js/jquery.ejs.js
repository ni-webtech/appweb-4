/*
    jquery.ejs.js - Ejscript jQuery support
    http://www.ejscript.com/
  
    Copyright (c) 2009 Embedthis Software
    Dual licensed under GPL licenses and the Embedthis commercial license.
    See http://www.embedthis.com/ for further licensing details.
 */

function breakpoint(text) {
    log("Breakpoint");
}

function log(text) {
    $('#console').append('<div>' + new Date() + ' ' + text + '</div>');
}

function dump(o) {
    $.each(elts, function(key, value) { 
        log(key + ': ' + value); 
    });
}

(function($) {

    /* Non chainable functions */
    $.extend({
        elog: function(msg) {
            if (window.console) {
                console.debug(msg);
            } else alert(msg);
        }
    });

    /* Chainable functions */
    $.fn.extend({
        /*
            Table control client side support
         */
        eTable: function(settings) {

            //  MOB -- call table not $table
            var $table = $(this);

            /*
             *  Blend options with defaults and save
             */
            var options = { refresh: 60000, sort: null, sortOrder: "ascending" };
            $.extend(options, settings);
            options.dynamicUpdate = options.url ? true: false;

            $table.data("options", options);

            function applyTableData(data) {
                options.sortConfig = $table[0].config;
                $table.replaceWith(data);
                $table = $("#" + $table.get(0).id);
                $table.data("options", options);
                sortTable($table, options)
            };

            /*
                Timeout function to update the table data contents
             */
            function updateTable() {
                if (!options.dynamicUpdate) {
                    setTimeout(function() { updateTable.apply($table);}, options.refresh);
                } else {
                    $.ajax({
                        url: options.url,
                        cache: false,
                        type: "GET",
                        error: function (http, msg, e) { $.elog("Error updating table control: " + msg); },
                        success: function (data) { applyTableData(data); },
                        complete: function() {
                            setTimeout(function() { updateTable.apply($table);}, options.refresh);
                        },
                    });
                }
            };

            /*
                Get dynamic data immediately, but don't sort until the data arrives
             */
            sortTable($table, options);
            if (options.refresh > 0) {
                setTimeout(function() { updateTable.apply($table);}, options.refresh);
            } 
            return this;
        },

        /*
         *  Toggle dynamic refresh on/off
         */
        eTableToggleRefresh: function() {
            var options = $(this).data("options");
            options.dynamicUpdate = !options.dynamicUpdate;
            var image = $(".-ejs-table-download", $(this)).get(0);
            if (options.dynamicUpdate) {
                $.get(options.url, function (data) { applyTableData(data); });
                image.src = image.src.replace(/red/, "green");
            } else {
                image.src = image.src.replace(/green/, "red");
            }
            return this
        },

        /*
            Define table sort options. May be called in the initial page HTML or may be called via the Ajax response.
         */
        eTableSetOptions: function(settings) {
            var options = this.data("options");
            $.extend(options, settings);
            return this
        },

//////////////////////////////////

        triggerAndReturn: function (name, data) {
            var event = new $.Event(name);
            this.trigger(event, data);
            return event.result !== false;
        },

        //    MOB - rename request
        callRemote: function () {
            var el      = this,

                data    = el.is('form') ? el.serializeArray() : [],
                //  MOB -- only support one
                method  = el.attr('method') || el.attr('data-method') || 'GET',
                url     = el.attr('action') || el.attr('href') || el.attr('data-url') || el.attr('data-remote');

            if (url === undefined) {
              throw "No URL specified for remote call (action or href must be present).";
            }
            if (el.triggerAndReturn('ajax:before')) {
                //  MOB -- dataType should be configurable
                changeUrl(url);
                $.ajax({
                    url: url,
                    data: data,
                    MOBdataType: 'script',
                    type: method.toUpperCase(),
                    beforeSend: function (xhr) {
                        el.trigger('ajax:loading', xhr);
                    },
                    success: function (data, status, xhr) {
                        el.trigger('ajax:success', [data, status, xhr]);
                        var apply = el.attr('data-apply');
                        if (apply) {
                            $(apply).html(data)
                        }
                    },
                    complete: function (xhr) {
                        el.trigger('ajax:complete', xhr);
                    },
                    error: function (xhr, status, error) {
                        el.trigger('ajax:failure', [xhr, status, error]);
                    }
                });
            }
            el.trigger('ajax:after');
        }
    });

/////////////////////

    $('a[data-confirm],input[data-confirm]').live('click', function () {
        var el = $(this);
        if (el.triggerAndReturn('confirm')) {
            if (!confirm(el.attr('data-confirm'))) {
                return false;
            }
        }
    });

    $('table[data-remote] tr').live('click', function(e) {
        var table = $(this).parents("table");
        var id = $(this).attr('data-id');
        if (!id) {
            id = $(this).parent().children().index($(this)) + 1;
        }
        table.attr('data-url', table.attr('data-remote') + "?id=" + id);
        table.callRemote();
        e.preventDefault();
    });

    $('button[data-remote]').live('click', function(e) {
        $(this).callRemote();
        e.preventDefault();
    });

    // $('div.tabbed-dialog div.-ejs-tabs ul li').live('click', function(e) {
    $('div.tabbed-dialog li').live('click', function(e) {
        // $(this).attr('data-url', $(this).attr('click');
        breakpoint();
        var next = $(this).attr('data-remote');
        $('div[id|=pane]').hide();
        var pane = $('div#' + next);
        pane.fadeIn(750);
        pane.addClass('pane-visible');
        pane.removeClass('pane-hidden');
        e.preventDefault();
    });

    //  MOB -- just to ensure that click on submit buttons get added if remote
    $('input[type=submit]').live('click', function (e) {
        $(this).attr("checked", true);
    });

    $('form[data-remote]').live('submit', function (e) {
        $(this).callRemote();
        e.preventDefault();
    });

    $('a[data-remote],input[data-remote]').live('click', function (e) {
        $(this).callRemote();
        e.preventDefault();
    });

    $('a[data-method]:not([data-remote])').live('click', function (e){
        var link = $(this),
            href = link.attr('href'),
            method = link.attr('data-method'),
            form = $('<form method="post" action="'+href+'"></form>'),
            metadata_input = '<input name="_method" value="'+method+'" type="hidden" />';

        if (csrf_param && csrf_token) {
          metadata_input += '<input name="'+csrf_param+'" value="'+csrf_token+'" type="hidden" />';
          form.hide()
              .append(metadata_input)
              .appendTo('body')
              .submit();
        } else {
          throw "No CSRF token found (is csrf_meta_tag in your layout?).";
        }
        e.preventDefault();
    });

/////////////////////////////////////////
/*
    //  Address
    function changeUrl(i) {
        console.log("CHANGE " + i);
        $.address.title(i);
    }
    $.address.strict(false);
    $.address.externalChange(function(e) {
        console.log("EXT CHANGE " + e.value);
        changeUrl(e.value);
    });
    $.address.change(function(e) {
        console.log("INT CHANGE " + e.value);
        $.address.title(e.value);
    });
    $.address.init(function(e) {
        console.log("INIT CALLBACK");
    });
    $.address.title("MOB");
    $.address.value("ABC");
*/

/////////////////////////////////////////

    function sortTable($table, options) {
        if (options.sort) {
            if (!options.sortConfig) {
                var el = $("th", $table).filter(':contains("' + options.sort + '")').get(0);
                if (el) {
                    options.sortConfig = {sortList: [[el.cellIndex, (options.sortOrder.indexOf("asc") >= 0) ? 0 : 1]]};
                } else options.sortConfig ={sortList: [[0, 0]]};
            }
            $table.tablesorter(options.sortConfig);
        }
    }
})(jQuery);

