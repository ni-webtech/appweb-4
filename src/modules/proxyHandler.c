/* 
    proxyHandler.c -- Test handler prototype only

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.

    LoadModule proxyHandler mod_proxy
    AddHandler proxyHandler 

    ProxyPass /prefix http://ipaddr:port/uri timeout=NN keepalive=NN min=NN max=NN buffer=NN rewrite=headers,body
        /prefix creates a location block
        rewrite rewrites Header and body URIs to appear to come from the proxy. Default == headers

    Headers:
        X-Forwarded-For     IP address of the client
        X-Forwarded-Server  Hostname of the proxy server
        X-Forwarded-Host    The orignal host requested by the client in the Host header

    Design:
        Routed to proxy just like any other request
        
 */

/*********************************** Includes *********************************/

#include    "appweb.h"

#if BIT_PACK_PROXY
/************************************ Locals ***********************************/

/*********************************** Forwards *********************************/

/************************************* Code ***********************************/

static void openProxy(HttpQueue *q)
{
#if UNUSED
    if ((pp = mprAllocObj(Proxy, manageProxy)) == 0) {
    }
#endif
}


static void closeProxy(HttpQueue *q)
{
}


HttpConn *getConn(HttpConn *conn)
{
#if UNUSED
    HttpConn    *target;

    if ((target = mprPopItem(conn->proxies)) != 0) {
        return target;
    }
#endif
    return 0;
}


static void startProxy(HttpQueue *q)
{
#if UNUSED
    HttpConn    *conn, *target;
    MprHash     *hp;

    conn = q->conn;
    rx = conn->rx;
    loc = rx->loc;
    
    httpo
    uri = httpJoinUriPath(NULL, rx->parsedUri, loc->prefix);

    pp->target = getConnection(conn);

    - how to hook into the pipeline for the target

    if (httpConnect(pp->target, rx->method, httpUriToString(uri)) < 0) {
        for (mprGetFirstHash(rx->headers); hp; hp = mprGetNextHash(rx->headers, hp)) {
            httpSetHeader(target, hp->key, hp->data);
        }
        httpSetHeader(target, "X-Forwarded-Host", );
        httpSetHeader(target, "Host", );
        httpSetHeader(target, "X-Forwarded-Server", );
        httpSetHeader(target, "X-Forwarded-For", );
    }
    //  Push headers out - this may not work
    httpFlush();
#endif
}


static void readyProxy(HttpQueue *q)
{
    //  Anything to do here?
}


static int sofar;

static void processProxy(HttpQueue *q)
{
    int     count = 50000;
    
    q->max = 65536;
    
#if USE0
    /*
     May overflow q->max
     */
    while (sofar < count) {
        httpWrite(q, "%d aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", sofar++);
    }
    httpFinalize(q->conn);
    sofar = 0;
#endif
    
    
#if USE1
    httpWrite(q, "%d aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", sofar++);
    if (sofar == count) {
        httpFinalize(q->conn);
        sofar = 0;
    }
#endif
    
    
#if USE2
    while (sofar < count && q->count < (q->max * 3 / 4)) {
        /* NOTE: httpWrite may do internal flush if count > max */
        httpWrite(q, "%d aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n", sofar++);
    }
    if (sofar == count) {
        httpFinalize(q->conn);
        sofar = 0;
    }
#endif
    
    
#if USE3
    /*
        MANUAL FLOW CONTROL
     */
    HttpPacket  *packet;
    int         i, size;
    
    size = 1024;
    for (; sofar < count && q->count < q->max; sofar++) {
        packet = httpCreateDataPacket(size);
        for (i = 1; i < size - 1; i++) {
            mprPutCharToBuf(packet->content, 'a');
        }
        mprPutCharToBuf(packet->content, '\n');
        httpPutForService(q, packet, HTTP_DELAY_SERVICE);
    }
    if (sofar == count) {
        httpFinalize(q->conn);
        sofar = 0;
    }
#endif

#if USE4 || 1
    httpStealConn(q->conn);
#endif
}


static void incomingProxyData(HttpQueue *q, HttpPacket *packet)
{
    httpPutForService(q, packet, 1);
}


static void incomingProxyService(HttpQueue *q)
{
    HttpPacket  *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (packet->flags & HTTP_PACKET_DATA) {
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                mprLog(7, "IncomingProxyService downstream full, putback");
                httpPutBackPacket(q, packet);
                return;
            }
        }
        httpPutPacketToNext(q, packet);
    }
}


static void outgoingProxyData(HttpQueue *q)
{
#if UNUSED
    httpPutForService(q, packet, 1);
#endif
}


static void outgoingProxyService(HttpQueue *q)
{
    HttpPacket  *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (packet->flags & HTTP_PACKET_DATA) {
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                mprLog(7, "OutgoingProxyService downstream full, putback");
                httpPutBackPacket(q, packet);
                return;
            }
        }
        httpPutPacketToNext(q, packet);
    }
}

static ssize doOutput(HttpQueue *q, cchar *data, ssize len)
{
    HttpPacket  *packet;
    ssize       count;

    count = min(len, q->max - q->count);
    count = min(count, q->packetSize);
    packet = httpCreateDataPacket(count);
    mprPutBlockToBuf(packet->content, data, len);
    httpPutForService(q, packet, HTTP_SCHEDULE_QUEUE);
    return count;
}

static int proxyDirective(MaState *state, cchar *key, cchar *value)
{
#if UNUSED
    HttpRoute   *route;

    /*
        Syntax: Proxy URI_PATH URI [key=value]...
        Example:
        Proxy /prefix http://ipaddr:port/uri timeout=NN keepalive=NN min=NN max=NN buffer=NN rewrite=headers,body
     */
    if (maTokenize(state, value, "...", &mimeType, &program) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
#endif
    return 0;
}


int maProxyHandlerInit(Http *http, MprModule *module)
{
    HttpStage   *handler;
    MaAppweb    *appweb;

    if ((handler = httpCreateHandler(http, module->name, 0, module)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openProxy; 
    handler->close = closeProxy; 
    handler->incomingData = incomingProxyData; 
    handler->outgoingService = outgoingProxyService;
    handler->start = startProxy; 
    handler->ready = readyProxy; 
    handler->process = processProxy; 

    appweb = httpGetContext(http);
    maAddDirective(appweb, "Proxy", proxyDirective);
    return 0;
}
#else

int maProxyHandlerInit(Http *http, MprModule *mp)
{
    return 0;
}

#endif /* BIT_PACK_PROXY */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
