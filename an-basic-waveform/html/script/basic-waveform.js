var bw = {

    paused : false,

    lastDelay : 5000,

    togglePause : function (btn) {
        var self = this;

        if (self.paused) {
            btn.value = 'Pause';
            self.paused = false;
            self.refresh();
        } else {
            btn.value = 'Resume';
            self.paused = true;
        }

    },

    refreshPeriodChanged : function (e) {
    },

    refresh : function () {
        var self = this;

        var delay = 1 * $('#refreshPeriod').val();
        if (isNaN(delay) || delay < 100) {
            delay = self.lastDelay;
        } else {
            self.lastDelay = delay;
        }

        if (!self.paused && delay > 0) {
            console.debug('refresh ' + delay);
            setTimeout(function () {
                self.refresh();
            }, delay);
        }
    }

}

$(document).ready(function () {
    var divHeight = $('#msidenav').outerHeight() - 60;
    $('#controlPanel').height(divHeight);
    $('#chartPanel').height(divHeight);

    ru.loadGroupList(function (groups) {
        bw.refresh();
        // alert(groups.length);
        // alert(groups[0].channels);
    });
});
