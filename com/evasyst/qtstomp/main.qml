import QtQuick 2.6
import QtQuick.Window 2.2
import com.evasyst.QtStomp 4.6
import QtQuick.Controls 1.4
import "qrc:/com/evasyst/qtstomp/network.js" as NetworkService

Window {
    id: root
    visible: true
    width: 950
    height: 700
    title: qsTr("STOMP Debugger")

    Connections {
        target: QtStomp
        // set color-coded message output using html
        onMessageRecieved: {
            messagesField.append('<span style="color:'+_subscriptionColors[QtStomp.lastMessageSubscriptionId]+';">'+QtStomp.incomingMessage+'</span>');
        }
    }

    property var _token;
    // if authenticating
    property var _defaultHttpEndpoint: 'http://localhost:8080/api'
    // needs to be set to WS/WSS
    property var _defaultWsEndpoint: 'ws://localhost:8080/ws'
    property var _selectedSubscriptions: null
    property var _subscriptionColors: new Object();

    // select item
    function selectItem(subscriptionEndpoint){
        console.log('Selecting: '+subscriptionEndpoint)
        if(_selectedSubscriptions === null){
            _selectedSubscriptions = new Array();
        }
        _selectedSubscriptions.push(subscriptionEndpoint);

        console.log('Selected Subscription: '+_selectedSubscriptions)
    }

    function unselectItem(subscriptionEndpoint){
        console.log('Unselecting: '+subscriptionEndpoint)
        for(var i = 0; i <= _selectedSubscriptions.length-1; i++){
            if(_selectedSubscriptions[i] === subscriptionEndpoint){
                _selectedSubscriptions.splice(i,1);
            }
        }

        console.log('Selected Subscription: '+_selectedSubscriptions)
    }

    /**
     *  Takes a set of subscriptions with {0} limiter and creates
     *  appropriate subscription
     */
    function processSubscriptions(callback) {
        var _preparedSubscriptions = new Array()
        console.log('Process Subscriptions');
        if(_selectedSubscriptions.length === 0){
            console.log('NO selected subscription endpoints')
        }

        var subscriptionRecordIdsArray = new Array();
        // split the record ids
        if(subscriptionRecordIds.text !== ''){
            var toSplit = subscriptionRecordIds.text;
            subscriptionRecordIdsArray = toSplit.split(',');
            if(subscriptionRecordIdsArray.length === 0){
                console.log('NO selected subscription records')
            }
        }

        if(subscriptionRecordIdsArray.length > 0 && _selectedSubscriptions.length > 0) {
            for(var i = 0; i <= _selectedSubscriptions.length-1; i++){
                for(var j = 0; j <= subscriptionRecordIdsArray.length-1; j++){
                    if(_selectedSubscriptions[i].indexOf('{0}') > -1){
                        _preparedSubscriptions.push(_selectedSubscriptions[i].replace('{0}',subscriptionRecordIdsArray[j]));
                    }
                }
            }
        }

        console.log(_preparedSubscriptions);

        subscribeToEndPoints(_preparedSubscriptions);
    }

    /**
     *
     */
    function subscribeToEndPoints(_preparedSubscriptions){
        if(_preparedSubscriptions.length === 0){
            console.log('No subscriptions to subscribe to!')
        }

        for(var i = 0; i <= _preparedSubscriptions.length-1; i++){
            QtStomp.subscribeTo(_preparedSubscriptions[i])
            _activeSubscriptions.append({ 'path':_preparedSubscriptions[i], 'subId': QtStomp.subscriptionId})
            // generate random color for this subscription and it's messages
            var color = '#'+Math.floor(Math.random()*16777215).toString(16);
            console.log('SubId: '+QtStomp.subscriptionId+' color: '+color);
            _subscriptionColors[QtStomp.subscriptionId] = color;

        }
    }

    Rectangle{
        anchors.fill: parent

        Rectangle{
            id: connectionDetails
            height: 50
            anchors {
                top: parent.top
                left: parent.left

            }

            ComboBox {
                currentIndex: 0
                id: endpointSelector
                anchors {
                    top: parent.top
                    left: parent.left
                    topMargin: 5
                    leftMargin: 20
                }
                model: ListModel {
                    id: cbItems
                    ListElement { text: "Endpoint 1"; http: "http://localhost:8080/api"; wss: "ws://localhost:8080/ws" }
                }
                width: 200
                onCurrentIndexChanged: {
                    if(messagesField !== null) messagesField.append('Setting endpoint to: '+cbItems.get(currentIndex).text)
                    console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).http + ", " + cbItems.get(currentIndex).wss)
                    NetworkService._endpoint = cbItems.get(currentIndex).http
                    QtStomp.endpoint = cbItems.get(currentIndex).wss
                }
            }

            TextField {
                id: username
                placeholderText: "Username"
                anchors {
                    top: parent.top
                    left: endpointSelector.right
                    topMargin: 5
                    leftMargin: 20
                }
            }

            TextField {
                id: password
                placeholderText: "Password"
                anchors {
                    left: username.right
                    top: parent.top
                    topMargin: 5
                    leftMargin: 20
                }
                echoMode: TextInput.PasswordEchoOnEdit
            }

            Button {
                id: connectBtn
                text: "Connect"
                anchors {
                    left: password.right
                    top: parent.top
                    topMargin: 5
                    leftMargin: 20
                }
                onClicked: {
                    messagesField.append('Authenticating to: '+NetworkService._endpoint+'...')
                    NetworkService.login(username.text, password.text, function(){
                        _token = NetworkService._token;
                        messagesField.append('HTTPS Connection success, got token: '+NetworkService._token)
                        QtStomp.authorization = _token
                        QtStomp.openWebSocket()
                    });
                    endpointSelector.enabled = false
                    username.enabled = false
                    password.enabled = false
                    connectBtn.enabled = false
                    disconnectBtn.enabled = true
                }
            }

            Button {
                id: disconnectBtn
                text: "Disconnect"
                anchors {
                    left: connectBtn.right
                    top: parent.top
                    topMargin: 5
                    leftMargin: 20
                }
                enabled: false
                onClicked: {
                    messagesField.append('Disconnecting from: '+NetworkService._endpoint+'...')
                    _token = '';
                    QtStomp.authorization = _token
                    QtStomp.unsubscribeAll();
                    QtStomp.disconnectFromServer()

                    endpointSelector.enabled = true
                    username.enabled = true
                    password.enabled = true
                    connectBtn.enabled = true
                    disconnectBtn.enabled = false
                }
            }
        }

        /**
         *  Websocket Subscriptions
         */
        Rectangle {
            id: appSubscriptionSelection
            anchors{
                top: connectionDetails.bottom
                left: parent.left
                topMargin: 5
                leftMargin: 20
            }

            height: 150
            width: parent.width * .5
            ListModel {
                id: appSubscriptionModel
                ListElement {
                    name:'/app/someEndpointToSubscribeTo'
                }
                ListElement {
                    name:'/app/someOtherEndpointToSubscribeToWithId/{0}'
                }
            }

            ListView {
                id: appList
                anchors.fill: parent
                header: appSubscriptionSelectionHeader
                model: appSubscriptionModel
                clip: true
                delegate: Component {
                    Item {

                        width: parent.width
                        height: 20
                        Column {
                            CheckBox{
                                text: name
                                onCheckedChanged: {
                                    if(checked){
                                        selectItem(name)
                                    } else {
                                        unselectItem(name)
                                    }
                                }
                            }
                        }
                    }
                }
                focus: true
//                onCurrentItemChanged: console.log(model.get(list.currentIndex).name + ' selected')
            }

            Rectangle {
                id: customTopicEndpoint
                anchors {
                    top: appList.bottom
                    left: parent.left
                    topMargin: 10
                }

                Text {
                    id: appHeading
                    text: '/app/'
                    width: 35
                }

                TextField {
                    id: appSubscriptionInput
                    placeholderText: "Enter custom endpoint use {0} to denote record ids"
                    width: 200
                    anchors{
                        left: appHeading.right
                    }
                }

                Button{
                    text: 'Add'
                    anchors{
                        left: appSubscriptionInput.right
                        leftMargin: 10
                    }
                    onClicked: {
                        // can't figure out how to make these 'checked' by default
                        appSubscriptionModel.append({'name':'/app/'+appSubscriptionInput.text});
                    }
                }
            }

            Component{
                id: appSubscriptionSelectionHeader
                Rectangle{
                    id: banner
                    width: parent.width; height: 50
                    Text {
                        anchors.centerIn: parent
                        text: "App Topic Endpoints"
                        font.pixelSize: 12
                    }
                }
            }
        }
        /**
         *
         */
        Rectangle{
            id: topicSubscriptionSelection
            anchors{
                top: connectionDetails.bottom
                left: appSubscriptionSelection.right
                topMargin: 5
                leftMargin: 20
            }
            height: 150
            width: parent.width * .5
            ListModel {
                id: topicModel
                ListElement {
                    name:'/topic/someobject/{0}/'
                }
                ListElement {
                    name:'/topic/someobject/{0}/sub'
                }
                ListElement {
                    name:'/topic/someobject/{0}/status'
                }
            }

            ListView {
                id: topicList
                anchors.fill: parent
                header: topicSubscriptionSelectionHeader
                model: topicModel
                clip: true

                delegate: Component {
                    Item {
                        width: parent.width
                        height: 20
                        Column {
                            CheckBox{
                                text: name
                                onCheckedChanged: {
                                    if(checked){
                                        selectItem(name)
                                    } else {
                                        unselectItem(name)
                                    }
                                }
                            }
                        }
                    }
                }
                focus: true
//                onCurrentItemChanged: console.log(model.get(list.currentIndex).name + ' selected')
            }

            Component{
                id: topicSubscriptionSelectionHeader
                Rectangle{
                    id: banner
                    width: parent.width; height: 50
                    Text {
                        anchors.centerIn: parent
                        text: "Topic Endpoints"
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                id: customAppEndpoint
                anchors {
                    top: topicList.bottom
                    left: parent.left
                    topMargin: 10
                }

                Text {
                    id: topicHeading
                    text: '/topic/'
                    width: 35
                }

                TextField {
                    id: topicSubscriptionInput
                    placeholderText: "Enter custom endpoint use {0} to denote record ids"
                    width: 200
                    anchors{
                        left: topicHeading.right
                        leftMargin: 5
                    }
                }

                Button{
                    text: 'Add'
                    anchors{
                        left: topicSubscriptionInput.right
                        leftMargin: 10
                    }
                    onClicked: {
                        // can't figure out how to make these 'checked' by default
                        topicModel.append({'name':'/topic/'+topicSubscriptionInput.text});
                    }
                }
            }
        }
        /**
         *  Record Id Inputs
         */
        Rectangle {
            id: recordSubscriptionInputs
            anchors{
                top: appSubscriptionSelection.bottom
                left: parent.left
                topMargin: 50
                leftMargin: 20
            }

            TextField{
                id: subscriptionRecordIds
                placeholderText: "Enter record ids seperated by ,"
                width: 600
            }

            Button{
                text: 'Subscribe'
                anchors{
                    left: subscriptionRecordIds.right
                    leftMargin: 10
                }
                onClicked: processSubscriptions()
            }
        }

        /**
         *
         */
        Rectangle {
            id: activeSubscriptions
            width: 400
            height: 375
            anchors {
                left: messagesField.right
                leftMargin: 10
                top: recordSubscriptionInputs.bottom
                topMargin: 40
            }
            border{
                width: 1
                color: 'black'
            }

            ListModel{
                id: _activeSubscriptions
            }

            ListView {
                id: _activeSubscriptionsList
                height: 200
                width: parent.width
                anchors{
                    top: parent.top
                    left: parent.left
                    bottom: parent.bottom
                }
                clip: true
                model: _activeSubscriptions
                delegate: Component {
                    Item {
                        width: parent.width
                        height: 20
                        Column {
                            Text{
                                text: path+' '+subId
                                color: _subscriptionColors[subId]
                            }
                        }
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                _activeSubscriptionsList.currentIndex = index
                                console.log(index);
                                console.log('Clicked: '+path+' '+subId)
                                QtStomp.unsubscribeFrom(path)
                                for(var i =0; i <_activeSubscriptions.count; i++){
                                    if(i == _activeSubscriptionsList.currentIndex){
                                        _activeSubscriptions.remove(i);
                                    }
                                }
                            }
                        }
                    }
                }
                highlight: Rectangle {
                    color: "grey"
                }
            }
        }

        TextArea{
            id: messagesField
            width: 500
            height: 200
            anchors{
                top: recordSubscriptionInputs.bottom
                left: parent.left
                topMargin: 40
                leftMargin: 20
            }
            text: 'STOMP Debugger'
            textFormat: TextEdit.RichText
        }

        TextArea {
            id: sendmessageField
            width: 400
            height: 100
            anchors {
                top: messagesField.bottom
                left: parent.left
                topMargin: 10
                leftMargin: 20
            }
        }

        Button {
            id: sendMessageButton
            anchors {
                left: sendmessageField.right
                leftMargin: 10
                top: sendmessageField.top
                topMargin: 20
            }
            onClicked: {
                QtStomp.sendMessage(sendmessageField.text);
            }
            text: "Send"
        }
    }

    Component.onCompleted: {
        NetworkService._endpoint = _defaultHttpEndpoint
        QtStomp.endpoint = _defaultWsEndpoint
    }

    onClosing: {
        QtStomp.unsubscribeAll()
        QtStomp.disconnectFromServer()
    }
}
