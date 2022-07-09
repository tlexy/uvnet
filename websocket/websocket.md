## websocket

### 过程
1. 将两个信息合并起来并返回
Sec-Websocket-Key: dGhlIHNhbXBsZSBub25jZQ==
258EAFA5-E914-47DA-95CA-C5AB0DC85B11
将这两个字符连接起来，经过sha1加密，再经过base64编码，返回给客户端
将其写到Sec-WebSocket-Accept字段的值中返回给客户端

2. 注意子协议的选择，虽然这个字段是可选的
Sec-WebSocket-Protocol: chat

3. 客户端发送关闭请求，并等待服务器的返回

p20, 4.2 Server-Side Requirements

4. Data Framing
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len | Extended payload length       |
|I|S|S|S| (4)   |A| (7)         |       (16/64)                 |
|N|V|V|V|       |S|             | (if payload len==126/127)     |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
| Extended payload length continued, if payload len == 127      |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |                 Payload Data  |
+-------------------------------- - - - - - - - - - - - - - - - +
:                   Payload Data continued ...                  :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                   Payload Data continued ...                  |
+---------------------------------------------------------------+   

5. 只有收到fin为1时，才向上报告有数据到来
6. opcode为0时，也意味着继续有数据到来
7. fin为0并且opcode不为0，这是连续帧（分片）的开始，后续的帧opcode均为0，直到fin为1为止
8. 分片之间可能会有控制帧的到来
9. 关闭帧中可能会有body，body的内容描述的关闭的原因。body的前两个字节是状态码，两个字节之后是详细描述
10. 发送了关闭连接之后，不允许再发送帧
11. 在收到关闭帧之后，必须立刻发送一个关闭帧回去