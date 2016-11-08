define(['jquery'], function ($) {
	return {
		getData: function (data, callback) {
			var request = $.ajax({
				type: "get",
				async: false,
				url: data,
				dataType: "jsonp",
				jsonp: "callback"//服务端用于接收callback调用的function名的参数
			});
			request.done(function (result) {
				callback = numberAnimation(result.count);
			});
			request.fail(function (result) {
				callback = numberAnimation("00266581");
			 });
		}
	};
});


/**
 * 字符串左侧补字符方法
 * @param  {string} number 传入的字符串
 * @param  {int} length 需要设定的长度
 * @param  {string} char   补入的字符
 * @return {string}        补完后字符串
 */
function padLeft(number, length, char) {
	return (Array(length).join(char || "0") + number).slice(-length);
};

/**
 * 为banner中的数字添加CSS3动画
 * @param  {int} number 传入的数据
 */
function numberAnimation(number) {
	var cntDigits = $('.cntDigit');

	number = padLeft(number.toString(), 8);

	cntDigits.each(function (index, el) {
		$(el).addClass('cntDigit' + number[index]);
	});
}
    
