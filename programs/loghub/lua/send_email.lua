--lipengwei
--2015.9.21
--send email need install LuaSocket-2.0.2 
--return ture or false and errstr
local smtp = require 'socket.smtp'
module("send_email", package.seeall)

--send email
--subject:主题 body:内容 email:收件人
function sendemail(subject, body,email)
	local msg = {
		headers = {
			to = email,
			cc = '',--抄送
			subject = subject
		},
		body = body
	}
	local ok, err = smtp.send {
		from = '<loghubemail@daoke.me>',
		rcpt = email,
		source = smtp.message(msg),
		user = 'loghubemail@daoke.me',
		password = 'AB12ab',
		server = 'smtp.exmail.qq.com',
		port = 25,
	}
	return ok,err
end
