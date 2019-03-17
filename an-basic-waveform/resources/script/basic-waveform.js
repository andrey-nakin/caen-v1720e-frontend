var bw = {

    config : {
        runInfoRefreshPeriod : 1000,
        analyzerPort : null
    },

    digitizers : {},

    histograms : {
        WF : 'WF',
        TM : 'Time',
        AM : 'Amp'
    },

    state : {
        runNumber : null
    },

    loadEqupments : function () {
        return mjsonrpc_db_get_values([ "/Equipment" ]).then(function (rpc) {
            var res = {};
            var equip = rpc.result.data[0];
            var s = '';
            for (i in equip) {
                if (typeof (equip[i]) === 'object') {
                    res[i.toUpperCase()] = equip[i];
                }
            }
            return res;
        });
    },

    detectAnalyzerPort : function () {
        var self = this;

        return mjsonrpc_db_get_values([ "/Programs/an-basic-waveform" ]).then(
                function (rpc) {
                    var proginfo = rpc.result.data[0];
                    var cmd = proginfo["start command"];
                    var re = /\s\-r([\d]+)/;
                    var result = cmd.match(re);
                    if (result) {
                        self.config.analyzerPort = parseInt(result[1]);
                    }
                });
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
        parsedUrl.port = self.config.analyzerPort || parsedUrl.port + 1;
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

        for ( var dig in self.digitizers) {
            for (var i = 0; i < 8; i++) {
                self.initChannel(dig, i);
                self.initChannel(dig, i);
            }
        }
    },

    initControls : function () {
        var self = this, html = '';

        for ( var dig in self.digitizers) {
            html += '<h3>' + dig + '</h3>';
            html += '<div>';

            for (var ch = 0; ch < 8; ch++) {
                html += '<div class="bw-row">';
                html += '<div class="bw-cell bw-left-column">';
                html += '<span>CH ' + ch + ':</span>';
                html += '</div>';
                html += '<div id="bt-' + dig + '_' + ch
                        + '" class="bw-cell bw-right-column">';

                for ( var hist in self.histograms) {
                    html += '<input type="checkbox" id="ch-' + hist + '_' + dig
                            + '_' + ch + '"> ';
                    html += '<label for="ch-' + hist + '_' + dig + '_' + ch
                            + '">' + self.histograms[hist] + '</label> ';
                }

                html += '</div>';
                html += '</div>';
            }

            html += '</div>';
        }

        $('#accordion').html(html);
    },

    init : function () {
        var self = this;

        self.loadEqupments().then(function (es) {
            self.digitizers = es;
            self.initControls();
            self.initWidgets();
        }).then(function () {
            return self.detectAnalyzerPort();
        }).then(function () {
            setInterval(function () {
                self.getRunNumber().then(function (runNo) {
                    self.checkRunNumber(runNo)
                });
            }, self.config.runInfoRefreshPeriod);
        });
    }

}

$(document).ready(function () {
    bw.init();
});
