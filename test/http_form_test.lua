local head = "POST /weiboapi/v2/sendMultimediaPersonalWeibo HTTP/1.1\r\n" .. 
"Host: 127.0.0.1\r\n" ..
"Accept: */*\r\n" ..
"Accept-Encoding: gzip\r\n" ..
"User-Agent: Mozilla/5.0 (unknown-x86_64-linux-gnu) Siege/3.0.5\r\n" ..
"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryDcO1SS14036635565255320MTDfpuu1403663556kkU\r\n" ..
"Connection: close\r\n" ..
"Content-type: application/json\r\n" ..
"Content-length: 1760"


local body = '------WebKitFormBoundaryDcO1SS14036635565255320MTDfpuu1403663556kkU' ..
'Content-Disposition:form-data;name="receiverLatitude\r\n\r\n' ..
'32.34\r\n' ..
'------WebKitFormBoundaryDcO1SS14036635565255320MTDfpuu1403663556kkU\r\n' ..
'Content-Disposition:form-data;name="receiverLongitude"\r\n\r\n' ..
'123.45\r\n' ..
'------WebKitFormBoundaryDcO1SS14036635565255320MTDfpuu1403663556kkU\r\n' ..
'Content-Disposition:form-data;name="interval"\r\n\r\n' ..
'123\r\n' ..
'------WebKitFormBoundaryDcO1SS14036635565255320MTDfpuu1403663556kkU--\r\n'

function parse_form_data(head, body)

    local boundary = string.match(head, 'boundary=(..-)\r\n')

    -- keep compatibility
    local head = string.lower(head)
    local check_boundary = string.match(head, "content%-type:%s?multipart/form%-data;%s?boundary=(..-)\r\n")
    if not check_boundary then
        return nil
    end
    boundary = string.gsub(boundary, '[%^%$%(%)%%%.%[%]%*%+%-%?%)]', '%%%1')

    -- the single chunk in http body's content-type may have more value, but we don't care them, so we should use '.-' to escape it, the exactly content is after the '\r\n\r\n'
    local tab = {}
    local form_kv = '%-' .. boundary .. '\r\nContent%-Disposition:%s?form%-data;%s?name="([%w_]+)".-\r\n\r\n(.-)\r\n%-'
    for key,val in string.gfind(body, form_kv) do
        tab[ key ] = val
    end

    --local form_fv = '%-' .. boundary .. '\r\nContent%-Disposition:%s?form%-data;%s?name="([%w_]+)";%s?filename="([%w%._]+)"\r\nContent%-Type:%s?([%w/-]+).-\r\n\r\n(.-)\r\n%-'
    --如果存在Content-Transfer-Encoding: binary,需要修改表达式,添加.-   \r\n.-Content
    --2014-05-12 jiang z.s.   
    --2014-05-13 jiang z.s. filename 可能为中文名

    ------------------------------------------------------------------------------------------------------------------------
    ---- jiang z.s. 2014-06-24
    ---- 分割符号,严格使用body内的内容
    ---- 分割方式首位都添加分割符号,避免二进制文件存在%-,导致无法解析
    local tmp_body_boundary = string.sub(body, 1, string.find(body,"\r\n") - 1 )
    tmp_body_boundary = string.gsub(tmp_body_boundary, '[%^%$%(%)%%%.%[%]%*%+%-%?%)]', '%%%1')
    ------------------------------------------------------------------------------------------------------------------------

    local form_fv = tmp_body_boundary .. '\r\nContent%-Disposition:%s?form%-data;%s?name="([%w_]+)";%s?filename="(.-)"\r\n.-Content%-Type:%s?([%w/-]+).-\r\n\r\n(.-)\r\n' .. tmp_body_boundary
    for name,file,dtype,data in string.gfind(body, form_fv) do
        tab[ name ] = {file_name = file, data_type = dtype, data = data}
    end
    return tab
end

local abc = parse_form_data(head, body)
for k,v in pairs(abc) do
	print(k,v)
end
