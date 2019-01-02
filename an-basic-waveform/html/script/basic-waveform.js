$(document).ready(function () {
    var divHeight = $('#msidenav').outerHeight() - 60;
    $('#controlPanel').height(divHeight);
    $('#chartPanel').height(divHeight);

    ru.loadGroupList(function (groups) {
        alert(groups);
    });
});
