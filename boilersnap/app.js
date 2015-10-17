var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var request = require('request');
var XMLHttpRequest = require('xhr2');

var routes = require('./routes/index');
var users = require('./routes/users');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
//app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));
app.use(logger('dev'));
app.use(bodyParser.json({limit: '50mb'}));
app.use(bodyParser.urlencoded({ extended: false, limit: '50mb' }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', routes);
app.use('/users', users);

console.log('hey');

/**
json to hold user and image associated with that user
key is username;
values are the link to the image they need to get
and the username of the person who sent it
*/

var data = {};

/**
post request for sending snapchats
takes in json
needs a base64 image and username of the sender.
*/
app.post('/send', function(req, res, next)
{
  // console.log(req.body);
  var sendFrom = req.body.sendFrom;
  var sendTo = req.body.sendTo;
  // img is base64 image
  var img_left = req.body.image_left;
  var img_right = req.body.image_right;

  var link_left = "";
  var link_right = "";

  done = function(){
    // console.log("entered done");
    data[sendTo] = {
      "from":sendFrom,
      "image_left":link_left,
      "image_right":link_right
    };
    console.log("DATA:");
    console.log(data);
    res.send(link_left + "\n" + link_right);
    res.end();
  }

  req2 = function(){
    request({
      url: "https://api.imgur.com/3/upload",
      method: "POST",
      json: true,
      headers: {
        "Content-Type":"application/json",
        "Authorization":"Client-ID f948415a877272b"
      },
      body: {image: img_right}
    }, function(err, response, body){
      console.log("entered second request");
      link_right = response.body.data.link;
      console.log("right: " + link_right);
      done();
    });
  }

  request({
    url: "https://api.imgur.com/3/upload",
    method: "POST",
    json: true,
    headers: {
      "Content-Type":"application/json",
      "Authorization":"Client-ID f948415a877272b"
    },
    body: {image: img_left}
  }, function(err, response, body){
    console.log("entered first request");
    link_left = response.body.data.link;
    console.log("left: " + link_left)
    req2();
  });






  /**
  var Request = new XMLHttpRequest();

  Request.open('POST', 'https://api.imgur.com/3/upload');

  Request.setRequestHeader('Authorization', 'Client-ID f948415a877272b');
  Request.setRequestHeader('Content-Type', 'application/json');

  var link = "";

  Request.onreadystatechange = function () {
    console.log('entered request function');
    if (this.readyState === 4) {
      console.log('Status:', this.status);
      console.log('Headers:', this.getAllResponseHeaders());
      console.log('Body:', this.responseText);
    }
    console.log(this.responseText);
    link = unescape(JSON.parse(this.responseText).data.link);
    console.log("IMAGE LINK: " + link);
    data[sendTo] = {
      "from":sendFrom,
      "image":link
    };
    console.log(data);
  };


  var body = {
    'image': img
  };

  Request.send(JSON.stringify(body));
  */
  // console.log(data);
  // res.send('done!');
  // res.end();
});

/**
post request for recieving snapchats
takes in json
needs the username of the user to check for
*/
app.post('/get', function(req, res, next){
  console.log(req.body);
  console.log(data);
  var username = req.body.username;

  var link = data[username];
  var ret = "No snapchats found :(";
  if (link != null)
  {
    ret = link;
    delete data[username];
  }
  res.send(ret);
  res.end();
});


// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});

app.listen(3000);

module.exports = app;
