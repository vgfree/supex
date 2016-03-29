local msg = {
    -- 返回成功
    MSG_SUCCESS				                ={"0", "ok!"},
    MSG_SUCCESS_WITH_RESULT			        ={"0", "%s"},

    --> 参数错误
    MSG_ERROR_REQ_FAILED_GET_SECRET                     ={"ME01002", "appKey 不合法!"},

    MSG_ERROR_REQ_SIGN                                  ={"ME01019", "签名错误!"},

    MSG_ERROR_LOCATION					={"ME01111", "定位失败,请在公共道路上使用！"},

    --> 系统错误
    SYSTEM_ERROR                                        ={"ME01022", "当前系统繁忙，服务暂不可用!"},

    MSG_ERROR_REQ_ARG			                ={"ME01023", "%s 参数错误!"},
}

return msg
