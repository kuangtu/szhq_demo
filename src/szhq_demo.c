#include "mongoose.h"
#include "szhq_demo.h"
#include "buffer.h"
#include "global.h"
#include "packet.h"

//MDGW地址、端口信息
static const char *s_conn = "tcp://IP:port";  

// client resources
static struct c_res_s {
  int i;
  struct mg_connection *c;
} c_res;

bool bRes = false;
bool bLogin = false;

//行情数据接收buffer
NET_BUFFER_T tBuf;

// CLIENT event handler
static void cfn(struct mg_connection *c, int ev, void *ev_data) {
  int *i = &((struct c_res_s *) c->fn_data)->i;
  if (ev == MG_EV_OPEN) {
    MG_INFO(("szhq client has been initialized"));
  } else if (ev == MG_EV_CONNECT) {
    MG_INFO(("szhq client connected"));

    //连接之后，发送登录消息
    //设置接入号、协议版本、发送心跳时间间隔
    GenerateLogonMsg();
    //发送登录消息
    mg_send(c, &gLogonMsg, sizeof(gLogonMsg));

  } else if (ev == MG_EV_READ) {
    struct mg_iobuf *r = &c->recv;
    //MG_INFO(("CLIENT got data: %.*s", r->len, r->buf));
    //（1）将接收到的数据存放到buffer中
    //（2）先要判断登录验证是否通过
    FeedRawPkt(&tBuf, r->buf, r->len);
    // 设置buffer中的len为0，读取数据完成
    r->len = 0;  // Tell Mongoose we've consumed data

  } else if (ev == MG_EV_CLOSE) {
    MG_INFO(("CLIENT disconnected"));
    // signal we are done
    ((struct c_res_s *) c->fn_data)->c = NULL;
  } else if (ev == MG_EV_ERROR) {
    MG_INFO(("CLIENT error: %s", (char *) ev_data));
  }
  else if (ev == MG_EV_RESOLVE)
  {
      MG_INFO(("CONNECTING RESOLVE..."));
  }
  else
  {

  }
}

static void
timer_reconn_fn(void* arg)
{
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    struct mg_connection *c = mgr->conns;

    //VSS连接了一个MDGW，从事件管理器中获取连接
    if (c == NULL)
    {
        c = mg_connect(mgr, s_conn, cfn, &c_res);
    }
}


static void
timer_hb_fn(void* arg)
{
    struct mg_mgr *mgr = (struct mg_mgr *) arg;
    //从事件管理器中，获取已有的连接
    struct mg_connection *c = mgr->conns;
    GenHbMsg(&tMsgHB);

    if (c != NULL && c->is_connecting == 0)
    {
        //mg_send不会直接执行socket的send操作
        //会先放到该connection的发送缓存buffer中
        //在mg_mgr_poll中检测发送状态，然后发送
        mg_send(c, &tMsgHB, sizeof(tMsgHB));
    }

}


int main(void) {
  //事件管理器
  struct mg_mgr mgr;  
  //连接信息
  struct mg_connection *c;

  //初始行情接收buffer
  InitBuf(&tBuf);

  //设置日志级别
  mg_log_set(MG_LL_INFO);  
  
  //初始化事件管理器
  mg_mgr_init(&mgr);        
  
  //发起TCP连接，此时需要注意的是:
  //（1）socket创建的时候为非阻塞模式
  //（2）connect之后，立即返回-1，但是errno设置为
  //（3）此时connection结构体中的is_connecting = 1，正在连接
  //（4）可能会马上成功
  //（5）实际连接成功的验证，是在mg_mgr_poll中进行判断
  if (c_res.c == NULL) {
    c_res.i = 0;
    c_res.c = mg_connect(&mgr, s_conn, cfn, &c_res);
    if (!c_res.c)
    {
        MG_INFO(("CLIENT %s", c_res.c ? "connecting" : "failed"));
    }
  }


  //设置timer，定时发送心跳消息
  mg_timer_add(&mgr, 5000, MG_TIMER_REPEAT, timer_hb_fn, &mgr);
  mg_timer_add(&mgr, 5000, MG_TIMER_REPEAT, timer_reconn_fn, &mgr);

  while (true)
  {
    mg_mgr_poll(&mgr, 100);  // Infinite event loop, blocks for up to 100ms
                             // unless there is network activity
  }
  mg_mgr_free(&mgr);         // Free resources
  return 0;

}
