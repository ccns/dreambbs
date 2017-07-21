$().ready(function () {
    var fontsize = parseInt((screen.width - 40) / 80);
    if (fontsize > 15) {
        fontsize = 15
    };
    $('pre').css('font-size', fontsize * 2 + 'px').css('line-height', fontsize * 2 + 'px');
    $('div#pbody').css('width', fontsize * 80 + 'px');
    $('div#header').css('width', fontsize * 78 + 'px').css('height', fontsize * 6 + 'px');
    $('.dcw').css('width', fontsize + 'px');
});
