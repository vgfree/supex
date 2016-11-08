/**
 * Created by helibin on 2015/9/22.
 */

requirejs.config({
        baseUrl: 'js/lib'
});
require(['jquery', 'function', '../roadSpeed', '../getCount', 'transform', 'juicer'], function ($, funObj, roadSpeed, getCount) {
        var limit_t, ac, value, mapObj, chinaDistrict, jqueryDom = {
                $doc: $(document),
        $title: $('#title'),
        $waiteBox: $('#waiteBox'),
        $cityName: $('#cityName'),
        $cityCode: $('#cityCode'),
        $city: $('#city'),
        $provinceList: $('#provinceList'),
        $cityList: $('#cityList')
        }, isNull, count = 0;
        $(function () {
                //fQueryDevice();
                fGetURL();
                function say()
        {
                getRoadData(310100, 0);
        }
        limit_t = setInterval(function(){say();}, 10000);
        });
        //limit_t = setInterval("fGetURL()", 10000);
        jqueryDom.$doc.on('click', '#refresh', function () {
                //jqueryDom.$waiteBox.css('display', 'block');
                fGetCityRoadData();
        });
        /*
        //查询设备是否为手机
        function fQueryDevice() {
        if (navigator.appVersion.search('Mobile') != -1) {
//路况路线宽度
$('.showCount').css({
'position': 'fixed',
'bottom': 0,
'left': 0,
'right': 0,
'width': '100%',
'text-align': 'center'
});
$('.data').css({
'width': 'auto',
'margin': 0
})
$('.showCount .text-before, .showCount .text-after').css({
'top': '-5px',
'font-size': '1.4rem'
});
$('.holder .cntDigit').css({
'height': '1.85rem',
'width': '1.4rem'
});
$('#tip').css({
'top': '15px',
'right': 'auto',
'left': '15px'
});
$('#refresh').css('display', "block");
$('.optionpanel').css({
'position': 'fixed',
'display': 'block',
'bottom': 'auto',
'top': '65px',
left: '10px'
})
}
$('.optionpanel').css({
'position': 'fixed',
'display': 'block',
'bottom': '0'
});
$('#tip').css('display', 'block');
}
*/
function fGetURL() {
        /*var urlArray, params;
          urlArray = window.location.href.split('=');
          if (urlArray.length === 1) {
          window.location.replace(window.location.href + '?city=上海市');
          return;
          }
          fInitProvList();
          params = fCheckCityInfo(chinaDistrict, decodeURI(urlArray[1]));
          if (params === undefined) {
          jqueryDom.$title.text("城市不存在，请重新输入");
          return;
          }
          $("#provinceList option[value=" + String(params.cityCode).substr(0, 2) + "0000]").attr("selected", true);*/
        mapInit();
        getCityList();
}

function G(id) {
                return document.getElementById(id);
}
/*地图初始化*/
function mapInit(cityName) {
        mapObj = new BMap.Map("allmap");
        mapObj.centerAndZoom("上海", 12);
        //mapObj.setMapStyle({style: 'midnight'});//自定义地图风格
        mapObj.addControl(new BMap.MapTypeControl({anchor: BMAP_ANCHOR_TOP_LEFT, offset: new BMap.Size(10, 5)}));
        mapObj.addControl(new BMap.NavigationControl({anchor: BMAP_ANCHOR_TOP_RIGHT, offset: new BMap.Size(10, 60)}));
        /*添加平移缩放控件*/
        mapObj.enableScrollWheelZoom();
        /*启用滚轮放大缩小*/
        mapObj.disable3DBuilding();
        siteserch();
}

function siteserch() 
{
        ac = new BMap.Autocomplete(    //建立一个自动完成的对象
                        {"input" : "suggestId"
                                ,"location" : mapObj
                        });

        ac.addEventListener("onhighlight", function(e) {  //鼠标放在下拉列表上的事件
                var str = "";
                var _value = e.fromitem.value;
                var value = "";
                if (e.fromitem.index > -1) {
                        value = _value.province +  _value.city +  _value.district +  _value.street +  _value.business;
                }    
                str = "FromItem<br />index = " + e.fromitem.index + "<br />value = " + value;

                value = "";
                if (e.toitem.index > -1) {
                        _value = e.toitem.value;
                        value = _value.province +  _value.city +  _value.district +  _value.street +  _value.business;
                }    
                str += "<br />ToItem<br />index = " + e.toitem.index + "<br />value = " + value;
                G("searchResultPanel").innerHTML = str;
        });

        ac.addEventListener("onconfirm", function(e) {    //鼠标点击下拉列表后的事件
                var _value = e.item.value;
                myValue = _value.province +  _value.city +  _value.district +  _value.street +  _value.business;
                G("searchResultPanel").innerHTML ="onconfirm<br />index = " + e.item.index + "<br />myValue = " + myValue;

                setPlace();
        });
}

function setPlace(){
        //mapObj.clearOverlays();    //清除地图上所有覆盖物
        function myFun(){
                var pp = local.getResults().getPoi(0).point;    //获取第一个智能搜索的结果
                mapObj.centerAndZoom(pp, 18);
                mapObj.addOverlay(new BMap.Marker(pp));    //添加标注
        }
        var local = new BMap.LocalSearch(mapObj, { //智能搜索
                onSearchComplete: myFun
        });
        local.search(myValue);
}

$('#stylelist').on('change', function () {
        mapObj.setMapStyle({style: $(this).val()});
});

function fCheckCityInfo(data, cityName) {
        var i = 0, dataLen = data.length;
        for (; i < dataLen; i++) {
                /*判断是否是直辖市*/
                if (cityName === data[i].name || cityName === data[i].name.substr(0, 2)) {
                        isDirectCity = true;
                        name = data[i].name;
                        code = data[i].code;
                        return {
                                isDirectCity: isDirectCity,
                                        cityName: name,
                                        cityCode: code
                        };
                }
                var j = 0, listLen = data[i].list.length;
                for (; j < listLen; j++) {
                        if (cityName === data[i].list[j].name.substr(0, 2) || cityName === data[i].list[j].name) {
                                isDirectCity = false;
                                name = data[i].list[j].name;
                                code = data[i].list[j].code;
                                return {
                                        isDirectCity: isDirectCity,
                                                cityName: name,
                                                cityCode: code
                                };
                        }
                }
        }
}

/*初始化省份列表*/
function fInitProvList() {
        $.ajax({
                async: false,
        type: 'get',
        url: 'js/chinaDistrict.json',
        datatype: 'json'
        }).done(function (data) {
                chinaDistrict = data;
                var proBody = {data: data}, option = '{@each data as it}<option value="${it.code}">${it.name}</option>{@/each}';
                jqueryDom.$provinceList.append(juicer(option, proBody));
        });
}

/*初始化城市列表*/
function getCityList(data, cityCode) {
        /*var provCode = String(cityCode).substr(0, 2) + "0000";
          for (var key in data) {
          if (provCode === data[key].code) {
          body = {data: data[key].list};
          option = '{@each data as it}<option value="${it.code}">${it.name}</option>{@/each}';
          jqueryDom.$cityList.html(juicer(option, body));
          $("#cityList option[value=" + cityCode + "]").attr("selected", true);
          break;
          }
          }*/
        fGetCityRoadData();
}
/*
//省份改变时执行事件
jqueryDom.$doc.on('change', '#provinceList', function () {
var cityCode = $(this).val(), i = 0, len = chinaDistrict.length, body, option;
if (cityCode == 0) {
jqueryDom.$cityList.html('<option value="0">所属城市</option>').change();
return;
}
for (; i < len; i++) {
if (chinaDistrict[i].code == cityCode) {
cityList = chinaDistrict[i].list;
break;
}
}
body = {data: cityList};
option = '{@each data as it}<option value="${it.code}">${it.name}</option>{@/each}';
jqueryDom.$cityList.html(juicer(option, body));
jqueryDom.$cityList.children('option[value=' + jqueryDom.$cityCode.val() + ']').attr('selected', true);
jqueryDom.$cityList.change();
});

//城市改变后加载路况数据
jqueryDom.$doc.on('change', '#cityList', function () {
count = 0;
$('.showCount').css('display', 'none');
fGetCityRoadData();
});
*/
function fGetCityRoadData() {
        /*var provName = jqueryDom.$provinceList.find('option:selected').text(),
          provCode = jqueryDom.$provinceList.val(),
          cityName = jqueryDom.$cityList.find("option:selected").text(),
          cityCode = jqueryDom.$cityList.val(),
          i, len = 3;
          jqueryDom.$title.text("数据正在加载中……");
          jqueryDom.$waiteBox.css('display', 'block');
          history.pushState("", "page 2", "?city=" + cityName);
          */
        mapObj.clearOverlays();
        mapObj.centerAndZoom("上海", 12);
        getRoadData(310100, 0);
        //jqueryDom.$cityCode.val(cityCode);
}

/*获取城市路况*/
function getRoadData(cityCode, cursor) {
        roadSpeed.getRoadSpeed({
                cityCode: cityCode,
        sortMethod: 1,
        reqType: 0,
        cursor: cursor
        }, function (data) {
                if (data.ERRORCODE == "token_error") {
                        jqueryDom.$title.text("麻烦您刷新页面,以便正常使用");
                        return;
                }
                if (JSON.stringify(data.RESULT.trafficInfo).length === 2) {
                        //如果是直辖市则以最后一次的结果作为判断依据
                        if (jqueryDom.$provinceList.find('option:selected').text() === jqueryDom.$cityList.find("option:selected").text()) {
                                isNull = "ture";
                                if (count === 2 && isNull === "ture") {
                                        jqueryDom.$title.text("暂无相关数据...");
                                }
                                count++;
                        } else {
                                jqueryDom.$title.text("暂无相关数据...");
                        }
                        return;
                } else {
                        isNull = "false";
                }
                showCityRoadSpeed(data.RESULT.trafficInfo);
                //cursor不为0继续取值
                if (data.RESULT.cursor !== "0") {
                        jqueryDom.$title.text("数据正在加载中……");
                        jqueryDom.$waiteBox.css('display', 'block');
                        getRoadData(cityCode, data.RESULT.cursor);
                } else if (jqueryDom.$provinceList.find('option:selected').text() === jqueryDom.$cityList.find("option:selected").text()) {
                        if (count === 2) {
                                //$('.showCount').css('display', 'block');
                                //jqueryDom.$waiteBox.css('display', 'none');
                                //getCount.getData('http://www.daoke.fm/php/getCount.php');
                        }
                        count++;
                } else {
                        //$('.showCount').css('display', 'block');
                        //jqueryDom.$waiteBox.css('display', 'none');
                        //getCount.getData('http://www.daoke.fm/php/getCount.php');
                }
        }
        );
}

/*显示城市路况*/
function showCityRoadSpeed(jsonData) {
        var len = jsonData.length, i = 0;
        for (; i < len; i++) {
                jsonData[i][0] = wgs2bd(Number(jsonData[i].EB), Number(jsonData[i].EL));
                jsonData[i][1] = wgs2bd(Number(jsonData[i].SB), Number(jsonData[i].SL));
                addMarker(jsonData[i]);
        }
}

/*给图层增加覆盖物*/
function addMarker(data) {
        var opts, strokeColor, speed, polylineArr = [], polyline, infoWindow;
        /*构造折线对象*/
        strokeColor = '', speed = data.averageSpeed;
        /*根据速度设置标记的颜色*/
        /*if (speed >= 0 && speed < 5) {*/
        //strokeColor = '#751417';
        //} else if (speed >= 5 && speed < 10) {
        //strokeColor = '#cb2127';
        //} else if (speed >= 10 && speed < 20) {
        //strokeColor = '#d4642a';
        //} else if (speed >= 20 && speed < 30) {
        //strokeColor = '#db802c';
        //} else if (speed >= 30 && speed < 60) {
        //strokeColor = '#fff12c';
        //} else if (speed >= 60 && speed < 80) {
        //strokeColor = '#6eb352';
        //} else {
        //strokeColor = '#32a456';
        /*}*/
        if (speed >= 0 && speed < 5) {
                strokeColor = '#000000';
        } else if (speed >= 5 && speed < 20) {
                strokeColor = '#330000';
        } else if (speed >= 20 && speed < 40) {
                strokeColor = '#000099';
        } else if (speed >= 40 && speed < 60) {
                strokeColor = '#006666';
        } else {
                strokeColor = '#009900';
        }

        polylineArr.push(new BMap.Point(data[0].lng, data[0].lat));
        polylineArr.push(new BMap.Point(data[1].lng, data[1].lat));
        /*设置折线覆盖*/
        polyline = new BMap.Polyline(polylineArr, {
                id: 'line' + data.roadID,
                 strokeColor: strokeColor, /*线颜色*/
                 strokeOpacity: 0.8, /*线透明度*/
                 strokeWeight: 4, /*线宽*/
                 strokeStyle: "solid" /*线样式*/
        });
        mapObj.addOverlay(polyline);
        /*构造点击路况信息窗口对象*/
        opts = {
                title: "<h4>微密路况实时播-道客FM</h4>", /*信息窗口标题*/
                enableMessage: true /*设置允许信息窗发送短息*/
        };
        content = '<p>路名：' + data.roadName + '</p>\
                  <p>从 ' + data.crossBegin + ' 到 ' + data.crossEnd + '路段</p>\
                  <p>平均时速：' + data.averageSpeed + 'Km/h</p>\
                  <p>最高时速：' + data.maxSpeed + 'Km/h</p>\
                  <p>更新时间：' + funObj.todayTime(data.time) + '</p>';

        infoWindow = new BMap.InfoWindow(content, opts);
        /*设置弹窗属性和显示内容,参数opts可不填*/
        polyline.addEventListener("click", function (e) {
                var point = new BMap.Point(e.point.lng, e.point.lat);
                mapObj.openInfoWindow(infoWindow, point);
        });
}

});
