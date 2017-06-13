.pragma library

var _endpoint, _token, userObject = null

/**
 *  Use this method to login and obtain a token for connecting to a password
 *  protected STOMP service
 */
function login(username, password, cb) {

    var body = {
        username: username,
        password: password
    }

    var xhr = new XMLHttpRequest()
    xhr.open("POST", _endpoint + "/authenticate", true)
    xhr.setRequestHeader("Content-type", "application/json; charset=UTF-8");
    xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200 || xhr.status === 201) {

                console.log(xhr.responseText)
                // userObject = JSON.parse(xhr.responseText)
                // if ( userObject.id === null) {
                //     console.log("NetworkService: ERROR! User id not found");
                //     return cb(xhr.statusText)
                // }

                console.log(xhr.getAllResponseHeaders())
                // could also be in the body of the response (just format it to authenticate to QtStomp)
                _token = "Basic " + xhr.getResponseHeader("x-header-token")
                console.log("NetworkService: New token added", _token)
                return cb(_token)
            } else {
                console.log("NetworkService: ERROR! Return code " + xhr.status)
                return cb(xhr.statusText)
            }
        }
    }

    xhr.send(JSON.stringify(body));

}
