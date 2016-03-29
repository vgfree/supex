/**
 * Created by Administrator on 14-3-17.
 */
define(['jquery','wgs2mars.min'],function ($,param,param1){
    //时间处理
    function todayTime(param) {
        var t = setTimes(param);
        var startTime = t.slice(5, 10);
        var endTime = getTime(0).slice(5, 10);
        if (startTime == endTime) {
            return '今天 ' + t.slice(11);
        } else {
            return t.slice(5);
        }
    }
    //时间差
    function differenceTime(param){
        var tt = getTimestamp(0)-param;
        if(tt <= 5){
            return '刚刚';
        }else if(tt < 60 && tt  > 5){
            return Math.round(tt)+'秒前';
        }
        return Math.round(tt/60)+'分钟前';
    }
    //得到时间
    function getTime(param) {
        var now = new Date();
        now.setSeconds(now.getMinutes()+param*60);
        return now.getFullYear()+'-'+add0(now.getMonth()+1)+'-'+add0(now.getDate())+' '+add0(now.getHours())+':'+add0(now.getMinutes())+':'+add0(now.getSeconds());
    }
    function add0(param){
        return param<10?("0"+param):(param);
    }
    /*两个时间差*/
    function timeLag(param,param1){
        var str = new Date(Date.parse(param.replace(/\-/g,"/")));
        var end = new Date(Date.parse(param1.replace(/\-/g,"/")));
        return ((end-str)/60000);
    }
    //得到时间戳
    function getTimestamp(param){
        var now = new Date();
        if(param!=0){
            now.setSeconds(now.getMinutes()+param*60);
        }
        var commonTime = new Date(Date.UTC(now.getFullYear(),now.getMonth(),now.getDate(),now.getHours(),now.getMinutes(),now.getSeconds()));
        var humanDate = commonTime.getTime()/1000 - 8*60*60;
        return humanDate;
    }
    function secondToDate(param) {
        if (!param) {
            return 0;
        }
        var time = '';
        if (param >= 24 * 3600) {
            time += parseInt(param / (24 * 3600)) + '天';
            param %= 24 * 3600;
        }
        if (param >= 3600) {
            time += parseInt(param / 3600) + '小时';
            param %= 3600;
        }
        if (param >= 60) {
            time += parseInt(param / 60) + '分钟';
            param %= 60;
        }
        if (param > 0) {
         time += param + '秒';
         }
        return time;
    }
    //时间戳转化成标准时间
    function setTimes(param){
        var now = new Date(param * 1000);
        return now.getFullYear()+'-'+add0(now.getMonth()+1)+'-'+add0(now.getDate())+' '+add0(now.getHours())+':'+add0(now.getMinutes())+':'+add0(now.getSeconds());
    }
    function getTimeobj(param){
        var now = new Date(param * 1000);
        var date ={
            year:now.getFullYear(),
            month:add0(now.getMonth()+1),
            day:add0(now.getDate()),
            hour:add0(now.getHours()),
            minutes:add0(now.getMinutes()),
            seconds:add0(now.getSeconds())
        };
        return date;
    }
    function getDirection(param){
        var direction='';
        if(param<80 && param>=10 ){
            direction = '由西南向东北';
        }else if(param<=100 && param>=80){
            direction = '由西向东';
        }else if(param<170 && param>100){
            direction = '由西北向东南'
        }else if(param<=190 && param>=170){
            direction= '由北向南';
        }else if(param<260 && param>190){
            direction = '由东北向西南';
        }else if(param<=280 && param>=260){
            direction = '由东向西';
        }else if(param<350 && param>280){
            direction = '由东南向西北';
        }else if((param<=10 && param>0) || (param>=350 && param<=360) ){
            direction = '由南向北';
        }else{
            direction='无';
        }
        return direction;
    }
    function checkStr(param){
        var pttr = /select|insert|update|delete|drop|\'|\/\*|\*|\.\.\/|\.\/|union|into|load_file|outfile|script|<|>|"|'/;
        var res = param.toLowerCase().match(pttr);
        if (res == null && param) return true;
        return false;
    }
    function checkProbably(param){
        if(param.match(/^[1-9]\d*$/)) return true;
        return false;
    }
    function checkChannelNumber(param){
        if(param.match(/^[a-zA-Z]\w{4,16}$/)) return true;
        return false;
    }
    function getLonLat(info){
        var request = $.ajax({
            type:'POST',
            dataType:'json',
            data:{roadID:info.roadID},
            url:'/request/m_api/getRoadCoordinate'
        });
        request.done(function (data){
            if(data.ERRORCODE=="0"){
                var body={};
                body[0] = transformFromWGSToGCJ(Number(data.RESULT.startLongitude),Number(data.RESULT.startLatitude));
                body[1] = transformFromWGSToGCJ(Number(data.RESULT.endLongitude),Number(data.RESULT.endLatitude));
                body.averageSpeed = info.averageSpeed;
                body.maxSpeed = info.maxSpeed;
                body.roadID = info.roadID;
                body.updateTime = todayTime(info.time);
                addMarker(body);
            };
        });
    }
    function getRoadPlace(param){
        $.ajax({
            type : 'POST',
            data : {roadID:param.roadID},
            dataType : 'json',
            url : '/request/m_api/getRoadPlace'
        }).done(function (data) {
            var $place = $('#roadPlace[data="'+param.roadID+'"]');
            if(data.length){
                getLonLat(param);
                $place.text('位置：'+data[0].cityName+data[0].countyName);
            }
        });
    }
    function getLocation(it){
        if(it.longitude.length<6){
            return '无法定位';
        }else if(it.roadName){
            return  it.cityName +it.countyName + it.roadName + '|时速：'+it.speed+'km/h |方向：'+getDirection(it.direction);
        }
        $.ajax({
            type : 'POST',
            data : {longitude:it.longitude,latitude:it.latitude,fileID:it.fileID},
            dataType : 'json',
            url : '/request/m_api/getLocation'
        }).done(function (data) {
            var $location = $('.location-text[data-id="'+it.fileID+'"]');
            if(data.ERRORCODE=="0"){
                var info = data.RESULT;
                if(!info.cityName){
                    $location.text('无法定位');
                    return;
                }
                var text = info.cityName +info.countyName + info.roadName + '|时速：'+it.speed+'km/h |方向：'+getDirection(it.direction);
                $location.html('<a href="javascript:;" >'+text+'</a>');
                $location.attr('title',text);
            }else{
                $location.text('无法定位');
            }
        });
    }
    function getResCount(param){
        $.ajax({
            type : 'POST',
            data : {interactID:param},
            dataType : 'json',
            url : '/request/data/getLiveRes'
        }).done(function (data) {
            if(data[0].joinInCount == undefined){
                var $yes = $('#yesCount[data="'+param+'"]'),
                    $no = $('#noCount[data="'+param+'"]');
                $yes.text(data[0].yesCount);
                $no.text(data[0].noCount);
            }else{
                var $res = $('#resCount[data="'+param+'"]');
                $res.text(data[0].joinInCount);
            }
        }).fail(function (){
            var $res = $('em[data="'+param+'"]');
            $res.text(0);
        });
    }
    var operateBar = {
            hide:function (){
                var setTime = setTimeout(function (){
                    $('#recorderUpload').addClass('hide')
                        .children('.progress-bar').css({'width':'0%'});
                },3000);
            },
            show:function (color,text,width){
                $('#recorderUpload').removeClass('hide')
                    .children('.progress-bar').text(text)
                    .css({'background-color': color,'width':width+'%'});
            }
    };
    function getCookieVal(offset) {
        var endStr = document.cookie.indexOf(";", offset);
        if (endStr == -1) {
            endStr = document.cookie.length;
        }
        return document.cookie.substring(offset, endStr);
    }
    function getChannelID(name){
        var arg = name + "=";
        var aLen = arg.length;
        var cLen = document.cookie.length;
        var i = 0;
        while (i < cLen) {
            var j = i + aLen;
            if (document.cookie.substring(i, j) == arg) {
                return getCookieVal(j);
            }
            i = document.cookie.indexOf(" ", i) + 1;
            if (i == 0) break;
        }
        return;
    }
    var funObj = {
        todayTime:todayTime,
        differenceTime:differenceTime,
        getTime:getTime,
        timeLag:timeLag,
        getTimestamp:getTimestamp,
        secondToDate:secondToDate,
        setTimes:setTimes,
        getTimeobj:getTimeobj,
        getDirection:getDirection,
        checkStr:checkStr,
        checkChannelNumber:checkChannelNumber,
        checkProbably:checkProbably,
        getRoadPlace:getRoadPlace,
        operateBar:operateBar,
        getChannelID:getChannelID,
        getLocation:getLocation,
        getResCount:getResCount
    };
    return funObj;
});