var nu = {

    parseNumber : function (s) {
    	if (typeof(s) === 'number') {
    		return s;
    	}
        if (!s) {
            return 0;
        }
        if (s.substr(0, 2) == '0x') {
            return parseInt(s.substr(2), 16);
        }
        return parseInt(s);
    },

    lPad : function (v, len) {
        var result = '' + v;
        while (result.length < len) {
            result = '0' + result;
        }
        return result;
    }
    
};
