var ru = {

    rootanaUrl : '/basic-waveform/',

    getRootanaUrl : function (query) {
        return this.rootanaUrl + query;
    },

    loadGroupList : function (cb, scope) {
        var self = this;
        $.getJSON(self.getRootanaUrl('h.json'), function (data) {
            var files = self.findNodesByName(data, 'Files');
            if (files && files._childs && files._childs.length) {
                var groups = [];

                $.each(files._childs, function (i, f) {
                    if (f._childs && f._childs.length) {
                        $.each(f._childs, function (i, h) {
                            var s = h._name.split('_');
                            var gName = s[0];
                            var g = self.findGroupByName(groups, gName);
                            if (!g) {
                                groups.push({
                                    name : gName,
                                    title : h._title.split(',')[0],
                                    channels : []
                                });
                                g = groups[groups.length - 1];
                            }
                            g.channels.push(s[1]);
                        });
                    }
                });
                cb.call(scope || self, groups);
            }
        });
    },

    findGroupByName : function (groups, gName) {
        return groups ? groups.find(function (g) {
            return g.name === gName;
        }) : null;
    },

    findNodesByName : function (h, nodeName) {
        if (!h) {
            return null;
        }
        if (h._name === nodeName) {
            return h;
        }
        var self = this;
        return h._childs ? h._childs.find(function (v) {
            return self.findNodesByName(v, nodeName);
        }) : null;
    }

};
