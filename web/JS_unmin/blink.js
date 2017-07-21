$(function () {
    blinkIn();
});

function blinkIn() {
    $(".sbk").each(function () {
        $(this).css("color", $(this).css("background-color"));
    });
    setTimeout("blinkOut()", 1000);
}

function blinkOut() {
    $(".sbk").each(function () {
        $(this).css("color", "");
    });
    setTimeout("blinkIn()", 1000);
}
