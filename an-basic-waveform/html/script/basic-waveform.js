var bw = {

    config : {
        runInfoRefreshPeriod : 1000,
    },

    digitizers : {
        V1720 : true,
        V1724 : true
    },

    histograms : {
        WF : true,
        TM : true,
        AM : true

    },

    state : {
        runNumber : null
    },

    getRunNumber : function () {
        return mjsonrpc_db_get_values([ "/Runinfo" ]).then(function (rpc) {
            var runinfo = rpc.result.data[0];
            return runinfo["run number"];
        });
    },

    lPad : function (v, len) {
        var result = '' + v;
        while (result.length < len) {
            result = '0' + result;
        }
        return result;
    },

    itemName : function (hist) {
        return 'Files/output' + this.lPad(this.state.runNumber, 8) + '.root/'
                + hist;
    },

    histButton : function (dig, ch, hist) {
        return $('#ch-' + hist + '_' + dig + '_' + ch);
    },

    parseUrl : function (url) {
        var res = {
            protocol : 'http',
            host : '',
            port : 80,
            query : ''
        };

        url = '' + url;
        if (url.indexOf(':') > 0) {
            res.protocol = url.substr(0, url.indexOf(':'));
            url = url.substr(url.indexOf(':') + 3);
        }

        if (url.indexOf(':') > 0) {
            res.host = url.substr(0, url.indexOf(':'));
            res.port = parseInt(url.substring(url.indexOf(':') + 1, url
                    .indexOf('/')));
        } else {
            res.host = url.substr(0, url.indexOf('/'));
        }
        url = url.substr(url.indexOf('/'));

        res.query = url;

        return res;
    },

    makeUrl : function (parsedUrl) {
        return parsedUrl.protocol + '://' + parsedUrl.host + ':'
                + parsedUrl.port + parsedUrl.query;
    },

    reloadFrame : function () {
        var self = this;

        var parsedUrl = self.parseUrl(document.location);
        parsedUrl.port = parsedUrl.port + 1;
        parsedUrl.query = '';
        url = self.makeUrl(parsedUrl);
        url += '?browser=no';
        url += '&items=' + '[';
        
        var firstItem = true;
        for ( var dig in self.digitizers) {
            for (var ch = 0; ch < 8; ch++) {
                for ( var hist in self.histograms) {
                    if (self.histButton(dig, ch, hist).is(':checked')) {
                        if (firstItem) {
                            firstItem = false;
                        } else {
                            url += ',';
                        }
                        url += this.itemName(hist + '_' + dig + '_' + ch);
                    }
                }
            }
        }
        url += ']';
        url += '&monitoring=' + escape($("#refreshPeriod").val());
        url += '&layout=' + escape($("#layout").val());
        console.debug(url);

        $('#frame').attr('src', url)
    },

    checkRunNumber : function (runNo) {
        var self = this;
        if (self.state.runNumber !== runNo) {
            self.state.runNumber = runNo;
            self.forceReloadFrame();
        }
    },

    forceReloadFrame : function () {
        var self = this;
        setTimeout(function () {
            self.reloadFrame();
        });
    },

    initChannel : function (dig, ch) {
        var self = this;

        $('#bt-' + dig + '_' + ch).buttonset();

        for ( var hist in self.histograms) {
            self.histButton(dig, ch, hist).bind('change', function () {
                self.forceReloadFrame();
            });
        }
    },

    initWidgets : function () {
        var self = this;

        $("#refreshPeriod").selectmenu({
            change : function (event, ui) {
                self.forceReloadFrame();
            }
        });

        $("#layout").selectmenu({
            change : function (event, ui) {
                self.forceReloadFrame();
            }
        });

        $("#accordion").accordion({
            heightStyle : "content"
        });

        for (var i = 0; i < 8; i++) {
            self.initChannel('V1720', i);
            self.initChannel('V1724', i);
        }
    },

    init : function () {
        var self = this;

        self.initWidgets();

        setInterval(function () {
            self.getRunNumber().then(function (runNo) {
                self.checkRunNumber(runNo)
            });
        }, self.config.runInfoRefreshPeriod);
    }

}

$(document).ready(function () {
    bw.init();
});
