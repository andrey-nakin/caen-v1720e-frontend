var bw = {

    config : {
        runInfoRefreshPeriod : 1000,
        analyzerPort : null
    },

    digitizers : {},
    digitizerTestModes : {},

    histograms : {
        WF : 'WF',
        TM : 'Time',
        AM : 'Amp'
    },

    state : {
        runNumber : null,
        loadingChannelConfig : false,
        configuringChannel : false,
        configuringChannelQueued : false
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

    loadTestModes : function () {
        var self = this, paths = [], digs = [];
        for (dig in self.digitizers) {
            digs.push(dig);
            paths.push("/Equipment/" + dig + "/Settings");
        }
        return mjsonrpc_db_get_values(paths).then(function (rpc) {
            var res = {};
            for (var i = 0; i < rpc.result.data.length; i++) {
                var s = rpc.result.data[i];
                var tm = s["test_mode"];
                if (tm) {
                    self.digitizerTestModes[digs[i]] = true;
                }
            }
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

    getEquipStatistics : function () {
        var self = this;
        var keys = [], digs = [];
        for (dig in self.digitizers) {
            digs.push(dig);
            keys.push('/Equipment/' + dig + '/Statistics');
        }
        return mjsonrpc_db_get_values(keys)
                .then(
                        function (rpc) {
                            var html = '';

                            if (rpc && rpc.result && rpc.result.data
                                    && rpc.result.data.length) {
                                for (var i = 0; i < rpc.result.data.length; i++) {
                                    var stat = rpc.result.data[i];
                                    var eps = stat['events per sec.'];
                                    var epsNum = Number.parseFloat(eps);
                                    if (!isNaN(epsNum)) {
                                        html += '<div class="bw-row">';

                                        html += '<div class="bw-cell bw-left-column">';
                                        html += '<label class="ui-widget">'
                                                + digs[i] + '</label>';
                                        html += '</div>';

                                        html += '<div class="bw-cell bw-right-column">';
                                        html += '<label class="ui-widget">'
                                                + epsNum.toFixed(1)
                                                + ' events/sec</label>';
                                        html += '</div>';

                                        html += '</div>';
                                    }
                                }
                            }

                            $('#eventInfo').html(html);
                        });
    },

    parseNumber : function (s) {
        if (!s) {
            return 0;
        }
        if (s.substr(0, 2) == '0x') {
            return parseInt(s.substr(2), 16);
        }
        return parseInt(s);
    },

    channelSettingsKey : function (propName) {
        return "/Equipment/" + $("#device").val() + "/Settings/"
                + (propName ? propName : "");
    },

    loadChannelConfig : function () {
        var self = this;

        $("#dcOffset").spinner("disable");
        $("#triggerThreshold").spinner("disable");

        return mjsonrpc_db_get_values([ self.channelSettingsKey() ]).then(
                function (rpc) {
                    self.state.loadingChannelConfig = false;

                    var ch = $("#channel").val();
                    if (ch !== '') {
                        var settings = rpc.result.data ? rpc.result.data[0]
                                : null;
                        if (settings) {
                            var dco = settings["channel_dc_offset"];
                            if (dco && dco.length > ch) {
                                $("#dcOffset").spinner("enable");
                                $("#dcOffset").spinner("value",
                                        self.parseNumber(dco[ch]));
                            }

                            var tt = settings["trigger_threshold"];
                            if (tt && tt.length > ch) {
                                $("#triggerThreshold").spinner("enable");
                                $("#triggerThreshold").spinner("value",
                                        self.parseNumber(tt[ch]));
                            }
                        }
                    }
                });
    },

    forceLoadChannelConfig : function () {
        var self = this;
        if (!self.state.loadingChannelConfig) {
            self.loadChannelConfig();
            self.state.loadingChannelConfig = true;
        }
    },

    getChannelConfigurationFromWidgets : function (rpc) {
        var self = this, v, paths = [], values = [], ch = $("#channel").val();
        var settings = rpc.result.data ? rpc.result.data[0] : null;

        if (settings) {
            var dco = settings["channel_dc_offset"];
            if (dco && dco.length > ch) {
                v = parseInt($("#dcOffset").spinner("value"));
                if (!isNaN(v) && v >= 0) {
                    paths.push(self.channelSettingsKey("channel_dc_offset"));
                    dco[ch] = v;
                    values.push(dco);
                }
            }

            var tt = settings["trigger_threshold"];
            if (tt && tt.length > ch) {
                v = parseInt($("#triggerThreshold").spinner("value"));
                if (!isNaN(v) && v >= 0) {
                    paths.push(self.channelSettingsKey("trigger_threshold"));
                    tt[ch] = v;
                    values.push(tt);
                }
            }
        }

        return {
            paths : paths,
            values : values
        };
    },

    writeChannelConfiguration : function (data) {
        if (data.paths.length > 0) {
            return mjsonrpc_db_paste(data.paths, data.values);
        }
    },

    configureChannel : function () {
        var self = this;

        return mjsonrpc_db_get_values([ self.channelSettingsKey() ]).then(
                function (rpc) {
                    return self.getChannelConfigurationFromWidgets(rpc);
                }).then(function (data) {
            return self.writeChannelConfiguration(data);
        }).then(function () {
            if (self.state.configuringChannelQueued) {
                console.debug('is queued');
                self.state.configuringChannelQueued = false;
                return self.configureChannel();
            } else {
                self.state.configuringChannel = false;
            }
        });
    },

    forceConfigureChannel : function () {
        var self = this;

        if (!self.state.configuringChannel) {
            self.configureChannel();
            self.state.configuringChannel = true;
        } else {
            console.debug("forceConfigureChannel queued");
            self.state.configuringChannelQueued = true;
        }
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
            self.forceReloadFrame(1000);
        }
    },

    forceReloadFrame : function (delay) {
        var self = this;
        setTimeout(function () {
            self.reloadFrame();
        }, delay);
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

        $("#device").selectmenu({
            change : function (event, ui) {
                self.forceLoadChannelConfig();
            }
        });
        var html = '<option value=""></option>', num = 0;
        for (dig in self.digitizerTestModes) {
            html += '<option value="' + dig + '">' + dig + '</option>';
            num++;
        }
        $("#device").html(html);
        $("#device").selectmenu("refresh");

        if (num > 0) {
            $('#deviceSetup').show();
        } else {
            $('#deviceSetup').hide();
        }

        $("#channel").selectmenu({
            change : function (event, ui) {
                self.forceLoadChannelConfig();
            }
        });

        $("#dcOffset").change(function () {
            self.forceConfigureChannel();
        });

        $("#dcOffset").spinner({
            min : 0,
            max : 65535,
            step : 1,
            page : 100,
            disabled : true,
            change : function (event, ui) {
                self.forceConfigureChannel();
            },
            spin : function (event, ui) {
                self.forceConfigureChannel();
            }
        });

        $("#triggerThreshold").change(function () {
            self.forceConfigureChannel();
        });

        $("#triggerThreshold").spinner({
            min : 0,
            max : 16383,
            step : 1,
            page : 100,
            disabled : true,
            change : function (event, ui) {
                self.forceConfigureChannel();
            },
            spin : function (event, ui) {
                self.forceConfigureChannel();
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
        }).then(function () {
            return self.loadTestModes();
        }).then(function () {
            self.initControls();
            self.initWidgets();
        }).then(function () {
            return self.detectAnalyzerPort();
        }).then(function () {
            setInterval(function () {
                self.getRunNumber().then(function (runNo) {
                    self.checkRunNumber(runNo)
                }).then(function () {
                    self.getEquipStatistics();
                });
            }, self.config.runInfoRefreshPeriod);
        });
    }

}

$(document).ready(function () {
    bw.init();
});
