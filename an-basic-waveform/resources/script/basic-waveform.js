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

    loadExperiment : function () {
        var self = this;
        return odb.loadExperiment().then(function (exp) {
            self.config.experiment = exp;
            self.updateExperimentInfo();
            self.updateRunInfo();
        });
    },

    updateExperimentInfo : function () {
        var self = this;
        document.title = self.config.experiment['name'] + ' - '
                + document.title;
    },

    updateRunInfo : function () {
        var self = this, s = '';
        if (self.config.experiment && self.config.experiment['name']) {
            s = 'Exp ' + self.config.experiment['name'];
        }
        if (self.state.runNumber) {
            if (s) {
                s += ' / ';
            }
            s += 'run #' + self.state.runNumber;
        }
        $('#runInfo').text(s);
    },

    loadTestModes : function () {
        var self = this;
        return odb.loadBooleanSettings(self.digitizers, "test_mode").then(
                function (data) {
                    self.digitizerTestModes = data;
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

    getEquipStatistics : function () {
        var self = this;
        return odb
                .loadEqupmentStatistics(self.digitizers)
                .then(
                        function (data) {
                            var html = '';

                            for (var i = 0; i < data.data.length; i++) {
                                var stat = data.data[i];
                                var eps = stat['events per sec.'];
                                var epsNum = Number.parseFloat(eps);
                                if (!isNaN(epsNum)) {
                                    html += '<div class="bw-row">';

                                    html += '<div class="bw-cell bw-left-column">';
                                    html += '<label class="ui-widget">'
                                            + data.digs[i] + '</label>';
                                    html += '</div>';

                                    html += '<div class="bw-cell bw-right-column">';
                                    html += '<label class="ui-widget">'
                                            + epsNum.toFixed(1)
                                            + ' events/sec</label>';
                                    html += '</div>';

                                    html += '</div>';
                                }
                            }

                            $('#eventInfo').html(html);
                        });
    },

    channelSettingsKey : function (propName) {
        var ch = $("#channel").val();
        if (ch) {
            ch = ch.split(':')[0];
        }
        return odb.getEquipmentSettingsKey(ch, propName);
    },

    getCurrentChannelNo : function () {
        var s = $("#channel").val();
        if (s) {
            return parseInt(s.split(':')[1]);
        }
        return null;
    },

    setChannelWidgetValues : function (ch, settings) {
        var dco = settings["channel_dc_offset"];
        if (dco && dco.length > ch) {
            $("#dcOffset").spinner("enable");
            $("#dcOffset").spinner("value", nu.parseNumber(dco[ch]));
            $("#dcOffsetLabel").removeClass("bw-disabled");
        }

        var tc = settings["trigger_channel"];
        if (tc && tc.length > ch) {
            $("#triggerChannel").checkboxradio("enable");
            $("#triggerChannel").prop('checked', tc[ch]).checkboxradio(
                    'refresh');
            $("#triggerChannelLabel").removeClass("bw-disabled");
        }

        var tt = settings["trigger_threshold"];
        if (tt && tt.length > ch) {
            $("#triggerThreshold").spinner("enable");
            $("#triggerThreshold").spinner("value", nu.parseNumber(tt[ch]));
            $("#triggerThresholdLabel").removeClass("bw-disabled");
        }

        var st = settings["signal"] ? settings["signal"].threshold : null;
        if (st && st.length > ch) {
            $("#signalThreshold").spinner("enable");
            $("#signalThreshold").spinner("value", nu.parseNumber(st[ch]));
            $("#signalThresholdLabel").removeClass("bw-disabled");
        }
    },

    loadChannelConfig : function () {
        var self = this;

        $("#dcOffset").spinner("disable");
        $("#dcOffsetLabel").addClass("bw-disabled");

        $("#triggerChannel").checkboxradio("disable");
        $("#triggerChannelLabel").addClass("bw-disabled");

        $("#triggerThreshold").spinner("disable");
        $("#triggerThresholdLabel").addClass("bw-disabled");

        $("#signalThreshold").spinner("disable");
        $("#signalThresholdLabel").addClass("bw-disabled");

        return mjsonrpc_db_get_values([ self.channelSettingsKey() ]).then(
                function (rpc) {
                    self.state.loadingChannelConfig = false;

                    var ch = self.getCurrentChannelNo();
                    if (!isNaN(ch)) {
                        var settings = rpc.result.data ? rpc.result.data[0]
                                : null;
                        if (settings) {
                            self.setChannelWidgetValues(ch, settings);
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
        var self = this, v, paths = [], values = [], ch = self
                .getCurrentChannelNo();
        var settings = rpc.result.data ? rpc.result.data[0] : null;

        if (!isNaN(ch) && settings) {
            var dco = settings["channel_dc_offset"];
            if (dco && dco.length > ch) {
                v = parseInt($("#dcOffset").spinner("value"));
                if (!isNaN(v) && v >= 0) {
                    paths.push(self.channelSettingsKey("channel_dc_offset"));
                    dco[ch] = v;
                    values.push(dco);
                }
            }

            var val = settings["trigger_channel"];
            if (val && val.length > ch) {
                v = $("#triggerChannel").is(':checked');
                paths.push(self.channelSettingsKey("trigger_channel"));
                val[ch] = v;
                values.push(val);
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

            var st = settings["signal"] ? settings["signal"].threshold : null;
            if (st && st.length > ch) {
                v = parseInt($("#signalThreshold").spinner("value"));
                if (!isNaN(v) && v >= -100) {
                    paths.push(self.channelSettingsKey("signal/threshold"));
                    st[ch] = v;
                    values.push(st);
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

    itemName : function (hist) {
        return 'Files/output' + nu.lPad(this.state.runNumber, 8) + '.root/'
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
            self.updateRunInfo();
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

        $('#bt-' + dig + '_' + ch).controlgroup();

        for ( var hist in self.histograms) {
            self.histButton(dig, ch, hist).bind('change', function () {
                self.forceReloadFrame();
            });
        }
    },

    initWidgets : function () {
        var self = this;

        $("#refreshPeriod").selectmenu({
            width : null,
            change : function (event, ui) {
                self.forceReloadFrame();
            }
        });

        $("#layout").selectmenu({
            width : null,
            change : function (event, ui) {
                self.forceReloadFrame();
            }
        });

        $("#channel").selectmenu({
            width : null,
            change : function (event, ui) {
                self.forceLoadChannelConfig();
            }
        });
        var html = '<option value=""></option>', num = 0;
        for (dig in self.digitizerTestModes) {
            html += '<optgroup label="' + dig + '">';
            for (var ch = 0; ch < 8; ch++) {
                html += '<option value="' + dig + ':' + ch + '">' + dig + ' / '
                        + ch + '</option>';
            }
            num++;
            html += '</optgroup>';
        }
        $("#channel").html(html);
        $("#channel").selectmenu("refresh");

        if (num > 0) {
            $('#deviceSetup').show();
        } else {
            $('#deviceSetup').hide();
        }

        $("#dcOffset").change(function () {
            if (this.select) {
                this.select();
            }
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

        $("#triggerChannel").checkboxradio({
            icon : true,
            disabled : true
        }).bind('change', function () {
            self.forceConfigureChannel();
        })

        $("#triggerThreshold").change(function () {
            if (this.select) {
                this.select();
            }
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

        $("#signalThreshold").change(function () {
            if (this.select) {
                this.select();
            }
            self.forceConfigureChannel();
        });

        $("#signalThreshold").spinner({
            min : -100,
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
                            + '" class="bw-toggle-button">'
                            + self.histograms[hist] + '</label> ';
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

        self.loadExperiment().then(function () {
            return odb.loadEqupmentList();
        }).then(function (es) {
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
                odb.loadRunNumber().then(function (runNo) {
                    self.checkRunNumber(runNo)
                }).then(function () {
                    self.getEquipStatistics();
                });
            }, self.config.runInfoRefreshPeriod);
        });
    }

}
