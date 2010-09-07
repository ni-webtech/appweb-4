/*
    jquery.ejs.js - Ejscript jQuery support
    http://www.ejscript.com/
  
    Copyright (c) 2010 Embedthis Software
    Dual licensed under GPL licenses and the Embedthis commercial license.
    See http://www.embedthis.com/ for further licensing details.
 */

;(function($) {
    /* 
        Published interface:
            $.ejs
            $.ejs.defaults
     */
    var defaults = {
        "updating": true,
        "toggle": 27,
    }
    $.fn.extend({
        /*
            Load or reload ejs support. Can be called after adding new elements to a page.
         */
        ejs: function(options) {
            $.extend(defaults, options || {});
            $('[data-sort-order]').each(sort);
            $('[data-refresh]').each(refresh);
            $('[data-modal]').each(function() {
                $(this).modal({
                    escClose: false, 
                    overlayId: "-ejs-modal-background", 
                    containerId: "-ejs-modal-foreground",
                });
            });
        },
    });

    $.fn.ejs.defaults = defaults;

    /************************************************* Support ***********************************************/
    /*
        Background request using data-remote attributes. Apply the result to the data-apply (or current element).
     */
    function backgroundRequest() {
        var elt     = $(this),
            data    = elt.is('form') ? elt.serializeArray() : [],
            method  = elt.attr('method') || elt.attr('data-method') || 'GET',
            /* Data-url is a modified remote URL */
            //  MOB -- remove data-url
            url     = elt.attr('action') || elt.attr('href') || elt.attr('data-url') || elt.attr('data-remote');

        if (url === undefined) {
            throw "No URL specified for remote call";
        }
        elt.trigger('http:before');
        // MOB changeUrl(url);
        $.ajax({
            url: url,
            data: data,
            type: method.toUpperCase(),
            beforeSend: function (http) {
                elt.trigger('http:loading', http);
            },
            success: function (data, status, http) {
                elt.trigger('http:success', [data, status, http]);
                var apply = elt.attr('data-apply');
                if (apply) {
                    $(apply).html(data)
                }
            },
            complete: function (http) {
                elt.trigger('http:complete', http);
            },
            error: function (http, status, error) {
                elt.trigger('http:failure', [http, status, error]);
            }
        });
        elt.trigger('http:after');
    }

    function copyDataAttributes(from) {
        this.each(function() {
            var e = $(this);
            $.each($(from)[0].attributes, function(index, att) {
                if (att.name.indexOf("data-") == 0) {
                    if (e.attr(att.name) == null) {
                        e.attr(att.name, att.value);
                    }
                }
            });
        });
    }

    /*
        Get an object containing all data-attributes
     */ 
    function getDataAttributes(elt) {
        var elt = $(elt);
        var result = {};
        $.each(elt[0].attributes, function(index, att) {
            if (att.name.indexOf("data-") == 0) {
                result[att.name.substring(5)] = att.value;
            }
        });
        return result;
    }

    /*
        Debug logging
     */
    function log(msg) {
        // $('#console').append('<div>' + new Date() + ' ' + text + '</div>');
        if (window.console) {
            console.debug(msg);
        } else {
            alert(msg);
        }
    }

    /*
        Foreground request using data-* element attributes. This makes all elements clickable and supports 
        non-GET methods with security tokens to aleviate CSRF.
     */
    function request() {
        var el          = $(this);
        var method      = el.attr('method') || el.attr('data-method') || 'GET';
        var url         = el.attr('action') || el.attr('href') || el.attr('data-click');
        var key         = el.attr('data-key');
        var keyFormat   = el.attr('data-key-format');
        var params;

        if (url === undefined) {
            alert("No URL specified");
            return;
        }
        method = method.toUpperCase();
        if (key) {
            if (!keyFormat) {
                keyFormat = (method == "GET") ? "path" : null;
            }
        } else keyFormat = null;

        if (keyFormat == "path") {
            var keys = [];
            var split = key.split("&");
            for (i in split) {
                pair = split[i];
                keys.push(pair.split("=")[1]);
            } 
            if (keys.length > 0) {
                url = url + "/" + keys.join("/");
            }
        } else if (keyFormat == "query") {
            url = url + "?" + key;
        } else if (keyFormat == "body") {
            params = key.split("&");
        }
        if (method == "GET") {
            window.location = url;
        } else {
            var tokenName = $('meta[name=SecurityTokenName]').attr('content');
            var token = $('meta[name=' + tokenName + ']').attr('content');
            if (token) {
                var form = $('<form method="post" action="' + url + '"/>').
                    append('<input name="-ejs-method-" value="' + method + '" type="hidden" />').
                    append('<input name="' + tokenName + '" value="' + token + '" type="hidden" />');
                if (params) {
                    for (k in params) {
                        param = params[k];
                        pair = param.split("=")
                        form.append('<input name="' + pair[0] + '" value="' + pair[1] + '" type="hidden" />');
                    }
                }
                form.hide().appendTo('body').submit();
            } else {
                alert("Missing Security Token");
            }
        }
    }

    /*
        Apply table sorting. Uses tablesorter plugin.
     */
    function sort(options) {
        var elt = $(this);
        var o = $.extend({}, defaults, options, elt.data("ejs-options") || {}, getDataAttributes(elt));
        if (!o.sortConfig) {
            if (o.sort != "false") {
                var items = $("th", elt).filter(':contains("' + o.sort + '")');
                var sortColumn = $("th", elt).filter(':contains("' + o.sort + '")').get(0);
                if (sortColumn) {
                    var order = (o["sort-order"].indexOf("asc") >= 0) ? 0 : 1;
                    o.sortConfig = {sortList: [[sortColumn.cellIndex, order]]};
                } else {
                    o.sortConfig = {sortList: [[0, 0]]};
                }
            }
            elt.tablesorter(o.sortConfig);
        }
        elt.data("ejs-options", o);
        return this;
    }

    /*
        Refresh dynamic elements using a data-refresh attribugte
     */
    function refresh(options) {
        function anim() {
            var e = $(this);
            var o = e.data("ejs-options")
            var effects = o.effects;
            if (effects == "highlight") {
                e.fadeTo(0, 0.3);
                e.fadeTo(750, 1);
            } else if (effects == "fadein") {
                e.fadeTo(0, 0);
                e.fadeTo(1000, 1);
            } else if (effects == "fadeout") {
                e.fadeTo(0, 1);
                e.fadeTo(1000, 0);
            } else if (effects == "bold") {
                /* Broken */
                e.css("font-weight", 100);
                e.animate({"opacity": 0.1, "font-weight": 900}, 1000);
            }
        }

        function applyData(data, http) {
            var d, id, oldID, newElt, isHtml = false;

            var id = this.attr("id");
            var apply = this.attr('data-apply') || id;
            var e = (apply) ? $('#' + apply) : $(this);
            var o = e.data("ejs-options");
            var contentType = http.getResponseHeader("Content-Type") || "text/html";

            if (contentType == "text/html") {
                try {
                    /* Copy old element options and id */
                    data = $(data);
                    data.data("ejs-options", o);
                    data.attr("id", id);
                    copyDataAttributes.call(data, e);
                    isHtml = true;
                } catch (e) {
                    console.debug(e);
                }
            }
            if (isHtml) {
                if (e[0] && e[0].tagName == "TABLE") {
                    /* Copy tablesorter data and config and re-sort the data before displaying */
                    data.data("tablesorter", e.data("tablesorter"));
                    var config = data[0].config = e[0].config;
                    data.tablesorter({ sortList: config.sortList });
                }
                e.replaceWith(data);
                e = data;

            } else {
                d = e[0];
                if (d.type == "checkbox") {
                    if (data == e.val()) {
                        e.attr("checked", "yes")
                    } else {
                        e.removeAttr("checked")
                    }
                } else if (d.type == "radio") {
                    //  MOB BROKEN
                    if (data == e.val()) {
                        e.attr("checked", "yes")
                    } else {
                        e.removeAttr("checked")
                    }
                } else if (d.type == "text" || d.type == "textarea") {
                    e.val(data);
                } else if (d.tagName == "IMG") {
                    e.attr("src", data)
                    // var img = $('<img />').attr('src', data);
                } else {
                    e.text(data);
                }
            }
            anim.call(e);
            return e;
        }

        function update(options) {
            var elt = $(this);
            var o = $.extend({}, options, elt.data("ejs-options") || {}, getDataAttributes(elt));
            elt.data("ejs-options", o);
            if (o.updating) {
                var method = o["refresh-method"] || "GET";

                //  MOB - consider firing events - elt.trigger('http:complete', http);
                $.ajax({
                    url: o.refresh,
                    type: method,
                    success: function (data, status, http) { 
                        if (http.status == 200) {
                            elt = applyData.call(elt, data, http); 
                        } else if (http.status == 0) {
                            log("Error updating control: can't connect to server");
                        } else {
                            log("Error updating control: " + http.statusText + " " + http.responseText); 
                        }
                    },
                    complete: function() {
                        setTimeout(function(elt) { 
                            update.call(elt, o); 
                        }, o["refresh-period"], elt);
                    },
                    error: function (http, msg, elt) { 
                        log("Error updating control: " + msg); 
                    },
                });
            } else {
                setTimeout(function(elt) { update.call(elt, o);}, o["refresh-period"], elt);
            }
            if (!o.bound) {
                $(document).bind('keyup.refresh', function(event) {
                    if (event.keyCode == o.toggle) {
                        $('[data-refresh]').each(toggle);
                    }
                });
                o.bound = true;
            }
        }
        var elt = $(this);
        var refreshCfg = $.extend({}, defaults, options || {});
        update.call(elt, refreshCfg);
        return this;
    }

    /*
        Toggle dynmic refresh on or off
     */
    function toggle() {
        elt = $(this);
        var o = elt.data("ejs-options");
        if (o) {
            o.updating = !o.updating;
        }
        /* FUTURE
            var image = $(".-ejs-table-download", e.get(0));
            if (o.updating) {
                image.src = image.src.replace(/red/, "green");
            } else {
                image.src = image.src.replace(/green/, "red");
            }
        */
    }

    /***************************************** Live Attach to Elements ***************************************/

    /* Click with data-confirm */
    $('a[data-confirm],input[data-confirm]').live('click', function () {
        var elt = $(this);
        elt.bind('confirm', function (evt) {
            return confirm(elt.attr('data-confirm'));
        });
        elt.trigger('confirm');
    });

    /* Click with tabs */
//  MOB -- should this be this or div#-ejs-tabs
    $('div.tabbed-dialog li').live('click', function(e) {
        var next = $(this).attr('data-remote');
        $('div[id|=pane]').hide();
        var pane = $('div#' + next);
        pane.fadeIn(500);
        pane.addClass('pane-visible');
        pane.removeClass('pane-hidden');
        e.preventDefault();
        return false
    });

    /* Click in form. Ensure submit buttons are added for remote calls */
    $('input[type=submit]').live('click', function (e) {
        $(this).attr("checked", true);
    });

    /* Click on table data-remote="" */
    $('table[data-remote] tr').live('click', function(e) {
        var table = $(this).parents("table");
        //  MOB -- remove this and use data-key
        var id = $(this).attr('data-id');
        if (!id) {
            id = $(this).parent().children().index($(this)) + 1;
        }
        //  MOB 
        table.attr('data-url', table.attr('data-remote') + "?id=" + id);
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click with button data-remote */
    $('button[data-remote]').live('click', function(e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on form with data-remote (background) */
    $('form[data-remote]').live('submit', function (e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on link with data-remote (background) */
    $('a[data-remote],input[data-remote]').live('click', function (e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on link foreground with data-method */
    $('a[data-method]:not([data-remote])').live('click', function (e) {
        request.apply(this)
/* UNUSED
        var link = $(this);
        var href = link.attr('href');
        var method = link.attr('data-method');
        var tokenName = $('meta[name=SecurityTokenName]').attr('content');
        var token = $('meta[name=' + tokenName + ']').attr('content');
        if (token) {
            $('<form method="post" action="' + url + '"/>').hide().
                append('<input name="__method__" value="' + method + '" type="hidden" />').
                append('<input name="' + tokenName + '" value="' + token + '" type="hidden" />').
                appendTo('body').
                submit();
        } else {
            alert("Missing Security Token");
        }
*/
        e.preventDefault();
    });

    /* Click on anything with data-click */
    $('[data-click]').live('click', function (e) {
        request.apply(this);
        e.preventDefault();
    });

    $('[data-click]').live('mouseover', function (e) {
        var click = $(this).attr("data-click");
        if (click.indexOf("http://") == 0) {
            window.status = click;
        } else {
            var location = window.location;
            window.status = location.protocol + "//" + location.host + click;
        }
    });
    $('[data-click]').live('mouseout', function (e) {
            window.status = "";
    });

/////////////////////////////////////////
/*  MOB -- TODO - fix location 
    //  MOB -- rename change Address
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

    /**************************** Initialization ************************/

    $(document).ready(function() {
        $(document).ejs();
    });

})(jQuery);

