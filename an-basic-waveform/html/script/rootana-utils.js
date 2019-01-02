var ru = {

    rootanaUrl : '/basic-waveform/',

    getRootanaUrl : function (query) {
        return this.rootanaUrl + query;
    },

    loadGroupList : function (cb, scope) {
        var e = this;
        $.getJSON(e.getRootanaUrl('h.json'), function (data) {
            var files = e.findNodesByName(data, 'Files');
            if (files && files._childs && files._childs.length) {

            }
        });
    },

    findNodesByName : function (h, nodeName) {
        if (!h) {
            return null;
        }
        if (h._name === nodeName) {
            return h;
        }
        var e = this;
        return h._childs ? h._childs.find(function (v) {
            return e.findNodesByName(v, nodeName);
        }) : null;
    }

};
