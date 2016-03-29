# HTTP 各种参数列表

## 请求参数中的各个指标
以下面的请求为例

    GET /forums/1/topics/2375?page=1#posts-17408 HTTP/1.1\r\n

* 请求方法: GET
* HTTP 版本: HTTP/1.1
* query_string: `page=1`
* fragment: `posts-17408`
* request path: `/forums/1/topics/2375`
* request URL: `/forums/1/topics/2375?page=1#post-17408`

也即，request URL 中包含了 query string, fragment 以及 request path.

但有时候, 它们之间的关系没这么明显:

    GET /get_no_headers_no_body/world HTTP/1.1\r\n

此处没有 query string, 没有 fragment, 而且 request path 和 request URL 是一样的, 都是 `/get_no_headers_no_body/world`.

关于 fragment, 一般在 API 设计中很少见，参见其[wiki页面](http://en.wikipedia.org/wiki/Fragment_identifier)

## 关于 chunked body

所谓 chunked body 是指在 HTTP body 中, body 被分割成多个部分,各个部分都有一个长度标识, 例如下面的 HTTP 请求:

        POST /post_chunked_all_your_base HTTP/1.1\r\n
        Transfer-Encoding: chunked\r\n
        \r\n
        1e\r\n
        all your base are belong to us\r\n
        0\r\n
        \r\n

此处的 body 从 `1e` 开始, 此处的 `1e` 指 `all your base are belong to us` 的长度, 它是一个十六进制字符串. 后面的 `0` 即表示后面没有内容了, 也即 body 结束. 话说回来, 这样的 body 组织, 很 X 蛋, 建议不要使用.

关于 chunked body, 参见其 [wiki 页面](http://en.wikipedia.org/wiki/Chunked_transfer_encoding).
