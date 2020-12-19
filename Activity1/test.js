// let {EtherPortClient} = require('etherport-client');
//   let five = require('johnny-five');
//   let board = new five.Board({
//     port: new EtherPortClient({
//       host: '192.168.1.13',
//       port: 3030
//     }),
//     repl: true
//   });
  
  
//   board.on("ready", function(){
//     var led = new five.Led(4);
//     led.blink(700);
//   });
var PORT = process.env.PORT||8080;
var express = require('express');
var request = require('request');
var app = express();
var baseUrl = "https://smartbinhome.herokuapp.com/";
var mysql = require('mysql');
const path = require('path');
var con = mysql.createConnection({
  host: "remotemysql.com",
  user: "85oFV6Veiy",
  password: "Lw00hmh6eQ",
  database: "85oFV6Veiy",
  multipleStatements: true
});
app.use("/public", express.static(path.join(__dirname, 'public')));
app.set('views', './views');
app.set('view engine', 'pug');

// for parsing application/x-www-form-urlencoded
app.use(express.urlencoded({
    extended: true
}));

app.get('/', function(req, res) {
  var bindata,led;
    con.connect(function(err) {
      var sql = "SELECT * FROM bin_status ORDER BY date_created DESC; SELECT * FROM led_status LIMIT 1;SELECT status,date_created FROM bin_status ORDER BY date_created DESC LIMIT 1;";
      con.query(sql, [1, 2], function(err, results){
        res.render('index',{bin:results[0],led:results[1][0],status:results[2][0]});
      });
    });
});
app.use('/delete', function (req, res, next) {
  con.connect(function(err) {
    // console.log(req.query.id); -- for direct GET query 
    var sql = "DELETE FROM bin_status WHERE id ="+req.query.id+";";
    con.query(sql, function (err, result) {
      console.log("1 record deleted");
    });
  });
  res.redirect('/');
});
app.use('/ledstatus', function (req, res, next) {
	// var options = {
  //     url: baseUrl + "/ledstatus",
  //     method: 'GET',
  //     headers: {
  //   	  'User-Agent':       'Super Agent/0.0.1',
  //   	  'Content-Type':     'application/x-www-form-urlencoded'
	//     },
  //     qs: {'val': req.body.val},
  //     timeout:2000
  //   }
	// request(options, function (error, response, body) {
        
    // if (!error && response.statusCode == 200) {  
      con.connect(function(err) {
        var sql = "UPDATE servo_status SET status ="+0+";";
        con.query(sql, function (err, result) {
          console.log("1 record updated");
        });
      });
      con.connect(function(err) {
        var sql = "UPDATE led_status SET status ="+req.body.val+";";
        con.query(sql, function (err, result) {
          console.log("1 record updated");
        });
      });
      res.send("success");
  //     res.send(body);
  //   } else {
  //     console.log("ESP no route")
  //     res.send("No Success");
  //   }  
	// });
});
app.use('/servostatus', function (req, res, next) {
	// var options = {
  //     url: baseUrl + "/servostatus",
  //     method: 'GET',
  //     headers: {
  //   	  'User-Agent':       'Super Agent/0.0.1',
  //   	  'Content-Type':     'application/x-www-form-urlencoded'
	//     },
  //     qs: {'val': req.body.val},
  //     timeout:2000
  //   }
	// request(options, function (error, response, body) {
        
  //   if (!error && response.statusCode == 200) {  
  //     console.log("Success: " + body);
      con.connect(function(err) {
        var sql = "UPDATE servo_status SET status ="+req.body.val+";";
        con.query(sql, function (err, result) {
          console.log("1 record updated");
        });
      });
      res.send("success");
  //   } else {
  //     console.log("ESP no route")
  //     res.send("No Success");
  //   }  
	// });
});
app.use('/binstatus', function (req, res, next) {
  con.connect(function(err) {
    var sql = "UPDATE monitor_status SET current_status ="+req.body.val+";";
    con.query(sql, function (err, result) {
      console.log("1 record updated");
    });
  });
  res.send("success");
});
app.listen(PORT);
