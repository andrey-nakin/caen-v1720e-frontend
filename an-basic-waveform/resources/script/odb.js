var odb = {

    loadExperiment : function () {
        return mjsonrpc_db_get_values([ "/Experiment" ]).then(function (rpc) {
            return rpc.result.data[0];
        });
    },

    loadEqupmentList : function () {
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

    loadEqupmentStatistics : function (equpmentList) {
        var data = {}, keys = [];
        data.digs = [];

        for (dig in equpmentList) {
            data.digs.push(dig);
            keys.push('/Equipment/' + dig + '/Statistics');
        }

        return mjsonrpc_db_get_values(keys).then(function (rpc) {
            data.data = rpc.result.data;
            return data;
        });
    },

    loadRunNumber : function () {
        var self = this;

        return mjsonrpc_db_get_values([ "/Runinfo" ]).then(function (rpc) {
            var runinfo = rpc.result.data[0];
            return runinfo["run number"];
        });
    },

    loadBooleanSettings : function (equpmentList, propName) {
        var paths = [], digs = [], result = {};

        for (dig in equpmentList) {
            digs.push(dig);
            paths.push("/Equipment/" + dig + "/Settings");
        }

        return mjsonrpc_db_get_values(paths).then(function (rpc) {
            var res = {};
            for (var i = 0; i < rpc.result.data.length; i++) {
                var s = rpc.result.data[i];
                var tm = s[propName];
                if (tm) {
                    result[digs[i]] = true;
                }
            }
            return result;
        });
    },

    getEquipmentSettingsKey : function (equip, propName) {
        return "/Equipment/" + equip + "/Settings/"
                + (propName ? propName : "");
    }

};
