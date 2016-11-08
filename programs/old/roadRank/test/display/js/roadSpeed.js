define(['jquery'], function ($) {
        return {
                getRoadSpeed: function (data, callback) {
                        var request = $.ajax({
                                type: 'POST',
                                dataType: 'json',
                                //crossDomain:true,
                                data: data,
                                url: 'http://192.168.21.56:8088/roadRankapi/v2/newTest_v2'
                                //url: 'http://api.daoke.io/roadRankapi/v2/newTest'
                                //url: 'http://localhost:8080'
                        });
                        request.done(function (result) {
                                callback(result);
                        });
                }
        };
});
