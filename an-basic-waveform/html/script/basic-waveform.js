$(document).ready(function () {
    var divHeight = $('#msidenav').outerHeight() - 60;
    $('#controlPanel').height(divHeight);
    $('#chartPanel').height(divHeight);

    alert(77);

    ru.loadGroupList(function (groups) {
        alert(groups);
    });
});
