var config = {
  apiKey: "AIzaSyBe6TMN3t-smfPqjBO_A1gJD1JGOWfl5jU",
  authDomain: "humidtemp-59706.firebaseapp.com",
  databaseURL: "https://humidtemp-59706.firebaseio.com",
  projectId: "humidtemp-59706",
  storageBucket: "humidtemp-59706.appspot.com",
  messagingSenderId: "1072796099399"
};
firebase.initializeApp(config);

function zeroPad(num, numZeros) {
  var n = Math.abs(num);
  var zeros = Math.max(0, numZeros - Math.floor(n).toString().length );
  var zeroString = Math.pow(10,zeros).toString().substr(1);
  if( num < 0 ) {
      zeroString = '-' + zeroString;
  }
  return zeroString+n;
}

function onClickUnit(unit) {
  var ref_unit = database.ref('/ID').child(unit + '/Current');
  ref_unit.once('value',function (snapshot_id) {
    var data = snapshot_id.val()
    var center = new google.maps.LatLng(data['Latitude'], data['Longtitude']);
    map.panTo(center);
  });
}

var no_map = true;
var map = true;
var marker = [];
var infowindow = [];
var history_content = [];
var database = firebase.database();
var ref = database.ref('/ID');

var myTableDiv = document.getElementById("tbl");

var table = document.createElement('TABLE');
var data_plot = [];
var unit_key = [];
var flag_sort = ['Black', 'Red', 'Yellow', 'Green', 'White'];
setInterval(ShowData, 300000); // 5 Minutes

function ShowData()
{
  if (table.hasChildNodes()) {
    table.removeChild(table.childNodes[0]);
  }
  var tableBody = document.createElement('TBODY');
  table.appendChild(tableBody);

  var tr = document.createElement('TR');
  tableBody.appendChild(tr);

  var td = document.createElement('TH');
  td.width = '100';
  td.appendChild(document.createTextNode('Unit Name'));
  tr.appendChild(td);

  td = null;
  td = document.createElement('TH');
  td.width = '45';
  td.appendChild(document.createTextNode('Flag'));
  tr.appendChild(td);

  td = null;
  td = document.createElement('TH');
  td.width = '50';
  td.appendChild(document.createTextNode('Humid'));
  tr.appendChild(td);

  td = null;
  td = document.createElement('TH');
  td.width = '75';
  td.appendChild(document.createTextNode('Temp (°C)'));
  tr.appendChild(td);

  td = null;
  td = document.createElement('TH');
  td.width = '155';
  td.appendChild(document.createTextNode('Updated Time'));
  tr.appendChild(td);

  var str_content = '';

  for (var ff = 0; ff < flag_sort.length; ff++) {
    for (var ii = 0; ii < data_plot.length; ii++) {
      var data = data_plot[ii];
      var key = unit_key[ii];
      if (data['Flag'] == flag_sort[ff]){
        var latlng = {lat: data['Latitude'], lng: data['Longtitude']};
        if(no_map){
          map = new google.maps.Map(document.getElementById('map'), {zoom: 16, center: latlng});
          for (var jj = 0; jj < data_plot.length; jj++){
            data = data_plot[jj];
            latlng = {lat: data['Latitude'], lng: data['Longtitude']};
            marker[jj] = new google.maps.Marker({
              position: latlng,
              map: map,
              icon: 'flag_' + data['Flag'].toLowerCase() + '.png'
            });
  
            str_content = '<div align=left><strong>'+"UnitName: "+ data['UnitName'] +'</strong><br>'+
                              "Flag Color: <strong><font color=" + data['Flag'].toLowerCase() + ">"+data['Flag']+'</font></strong><br>'+
                              "Temperature: "+data['Temperature']+'°C'+'<br>'+
                              "Humid: "+data['Humid']+"%"+'<br>'+
                              "Heat Index: "+data['HID']+'<br>'+
                              "Water: "+data['Water']+'<br>'+
                              "last update: "+CanvasJS.formatDate( new_date, "DD MMM YY HH:mm:ss")+'</div>';
            history_content.push(str_content)
            infowindow[jj] = new google.maps.InfoWindow({
                    content: str_content
                  });
                
            infowindow[jj].open(map, marker[jj]);
          }

          data = data_plot[ii];
          latlng = {lat: data['Latitude'], lng: data['Longtitude']};
          no_map = false;
        }

        var y1 = parseInt(data['DateTime'].substring(0,4));
        var m1 = parseInt(data['DateTime'].substring(4,6))-1;
        var d1 = parseInt(data['DateTime'].substring(6,8));
        var H1 = parseInt(data['DateTime'].substring(9,11));
        var Mi1 = parseInt(data['DateTime'].substring(12,14));
        var S1 = parseInt(data['DateTime'].substring(15,17));
        var new_date = new Date(y1, m1, d1,H1,Mi1,S1);
        var currentdate = new Date(); 

        var isOnline = false;
        if ((currentdate - new_date) < 5*60*1000) // 5 Minutes
          isOnline = true;

        tr = null;
        tr = document.createElement('TR');
        tableBody.appendChild(tr);

        var link = document.createElement('a');
        link.setAttribute('href', "info.html?unit="+key);
        link.setAttribute('target', "_blank");

        td = null;
        td = document.createElement('TD');
        td.width = '100';
        link.appendChild(document.createTextNode(data['UnitName']));
        link.onclick = function() {onClickUnit(key)};
        td.appendChild(link);
        tr.appendChild(td);

        td = null;
        td = document.createElement('TD');
        td.width = '45';
        td.appendChild(document.createTextNode(data['Flag']));
        td.setAttribute("STYLE","background-color:" + data['Flag'].toLowerCase() + ";text-align: center;");
        if (data['Flag'].toLowerCase().localeCompare('black') === 0){
          td.setAttribute("STYLE","background-color:" + data['Flag'].toLowerCase() + ";color:white;text-align: center;");
        }
        tr.appendChild(td);

        td = null;
        td = document.createElement('TD');
        td.width = '50';
        td.appendChild(document.createTextNode(data['Humid']));
        tr.appendChild(td);

        td = null;
        td = document.createElement('TD');
        td.width = '75';
        td.appendChild(document.createTextNode(data['Temperature']));
        tr.appendChild(td);

        td = null;
        td = document.createElement('TD');
        td.width = '155';
        td.appendChild(document.createTextNode(CanvasJS.formatDate( new_date, "DD MMM YY HH:mm:ss")));
        if (isOnline){
          td.setAttribute("STYLE","color:green");
        }
        else{
          td.setAttribute("STYLE","color:red");
        }
        tr.appendChild(td);

        str_content = '<div align=left><strong>'+"UnitName: "+ data['UnitName'] +'</strong><br>'+
                              "Flag Color: <strong><font color=" + data['Flag'].toLowerCase() + ">"+data['Flag']+'</font></strong><br>'+
                              "Temperature: "+data['Temperature']+'°C'+'<br>'+
                              "Humid: "+data['Humid']+"%"+'<br>'+
                              "Heat Index: "+data['HID']+'<br>'+
                              "Water: "+data['Water']+'<br>'+
                              "last update: "+CanvasJS.formatDate( new_date, "DD MMM YY HH:mm:ss")+'</div>';

        if (str_content !== history_content[ii]){
          marker[ii].setMap(null);
          infowindow[ii].close();

          marker[ii] = new google.maps.Marker({
            position: latlng,
            map: map,
            icon: 'flag_' + data['Flag'].toLowerCase() + '.png'
          });

          history_content[ii] = str_content;

          infowindow[ii] = new google.maps.InfoWindow({
                  content: str_content
                });
              
          infowindow[ii].open(map, marker[ii]);
        }
      }
  }
  }
  myTableDiv.appendChild(table);
}

ref.on('value',function (snapshot) {
    data_plot = [];
    unit_key = [];
    snapshot.forEach(function(childSnapshot) {       
    var key = childSnapshot.key; 
    unit_key.push(key);    
    console.log(key);

    var db_id = database.ref('/ID').child(key + '/Current');
    db_id.on('value',function (snapshot_id) {
        var data = snapshot_id.val()
        data_plot.push(data);
    });
    });
    ShowData();
});
