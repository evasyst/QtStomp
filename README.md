# QtStomp #

Adapted from [https://github.com/grundleborg/QtStomp](https://github.com/grundleborg/QtStomp) - Thanks!

Implements a wrapper around original classes `QtStompWrapper.cls`

* Can be added to QML
* Headers should be the same as authentication to STOMP Websocket
`Authorization: Basic _TOKEN_`

STOMP appears to be most commonly associated with Java Spring/Websockets: 
https://docs.spring.io/spring/docs/current/spring-framework-reference/html/websocket.html

Of Note:

(Guides)[https://spring.io/guides/gs/messaging-stomp-websocket/] expect WS support in spring to use `.withSockJS()`, this conflicts with this implementation
as QtStomp uses a TCP-Socket so there is no HTTP->301 Upgrade request required, just open the TCP Socket directed at your endpoint and start sending encoded (STOMP Protocol messages)[https://stomp.github.io/]

## To Do ##
* Clean-up some of the more "complicated" methods / repetitive functions / as this was mostly implemented when I was relatively new to Qt.
* Implement client-side heartbeats loop (right now server just sends heart-beats)
* Libraritize classes for other uses
* tests most likely no longer work
* cmake is broken as well