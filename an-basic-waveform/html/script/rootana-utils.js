var ru = {

    rootanaUrl : 'http://localhost:8081/',

    getRootanaUrl : function (query) {
        return this.rootanaUrl + query;
    },

    loadGroupList : function (cb, scope) {
        $.getJSON(this.getRootanaUrl('h.json'), function (data) {
            alert(123);
        });

        // cb.call(scope || this, [ 1, 2, 3 ]);
    }

};
