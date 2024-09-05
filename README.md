# szhq_demo
深交所行情接入DEMO示例，基于`mongoose` 库进行说明，主要功能包括了：

- 连接
- 接收数据
- 定时发送心跳消息
- 断开重连

## 连接
用户VSS程序通过TCP方式，连接MDGW行情网关成功之后，发送登录消息LOGON。

## 接收数据
- 根据行情接入协议，MDGW行情网关接收登录消息LOGON后，会发送登录消息到VSS。
- 接收行情数据，DEMO示例中通过文件保存。

## 发送心跳消息
VSS和MDGW之间需要定时发送心跳消息，通过定时器发送心跳消息。

## 连接检查
如果因为网络



# 参考：
【1】
【2】https://github.com/cesanta/mongoose