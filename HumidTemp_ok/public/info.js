var queryString = decodeURIComponent(window.location.search);
queryString = queryString.substring(1);
var queries = queryString.split("=");
var key = queries[1]

var config = {
  apiKey: "AIzaSyA8hvIgXiCLMJt0cLKtQI-mwge4WvQbIuo",
  authDomain: "humidtemp-59706.firebaseapp.com",
  databaseURL: "https://humidtemp-59706.firebaseio.com",
  projectId: "humidtemp-59706",
  storageBucket: "humidtemp-59706.appspot.com",
  messagingSenderId: "1072796099399"
};
firebase.initializeApp(config);

var Last_1 = 1 * 60 * 60 * 1000;
var Last_3 = 3 * 60 * 60 * 1000;
var Last_6 = 6 * 60 * 60 * 1000;
var Last_12 = 12 * 60 * 60 * 1000;
var Last_24 = 24 * 60 * 60 * 1000;
var Last_w = 7 * 24 * 60 * 60 * 1000;
var Last_m = 30 * 24 * 60 * 60 * 1000;
var plot_last = Last_24;
var title_idx = 4;

var database = firebase.database();

var curr_ref = database.ref('/ID').child(key + '/Current');
curr_ref.once('value',function (snapshot) {
  var data = snapshot.val()
  document.getElementById("unitname").innerHTML = 'Unit: ' + data['UnitName'];
});

function toggleDataSeries(e) {
	if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
		e.dataSeries.visible = false;
	}
	else {
		e.dataSeries.visible = true;
	}
	chart.render();
}

var ref = database.ref('/ID').child(key + '/Log');

var myTableDiv = document.getElementById("tbl");

var table = document.createElement('TABLE');

var data_plot = [];

var chart = new CanvasJS.Chart("chartContainer", {
	title:{
    text: "Humid & Temperature",
    fontSize: 20
  }
});
chart.render();

function stripLineHandler(position){
  if(!chart.options.axisX){
    chart.options.axisX ={};
  }
  if(!chart.options.axisX.stripLines){
    chart.options.axisX.stripLines = [];
  }
  chart.options.axisX.stripLines[0] = {
    value : position,
    thickness: 2,
    color: "#818181",
    showOnTop: true,
    label: (CanvasJS.formatDate( new Date(position), "DD MMM YY HH:mm:ss")),
    labelFontColor: "#333",
    // label: (new Date(position).getDate()+"-"+new Date(position).getHours()+":"+new Date(position).getMinutes()+":"+new Date(position).getSeconds()),
    labelPlacement: "outside"
  }
  chart.render();
 }

var prevXData = null;

document.getElementById("chartContainer").onmousemove = function (){
	if( document.getElementById("xData") ){	
    var currentXData = new Date( document.getElementById("xData").innerHTML ).getTime();
    if(prevXData !== currentXData){
      stripLineHandler(currentXData);
      prevXData = currentXData;
    }
  }
}

var x_dropdown = "Last Hour,Last 3 Hours,Last 6 Hours,Last 12 Hours,Last 24 Hours,Last Week, Last Month";
var chart_last_title = ["Last Hour","Last 3 Hours","Last 6 Hours","Last 12 Hours","Last 24 Hours","Last Week", "Last Month"];
var options = x_dropdown.split(",");
var select = document.getElementById('myoptions');
select.options[0] = new Option('Choose View', '');
for(var i=0; i<options.length; i++)
  select.options[i+1] = new Option(options[i], i);

function UpdateChart(){
  title_idx = document.getElementById('myoptions').selectedIndex - 1;
  if (title_idx == 0)
    plot_last = Last_1;
  else if (title_idx == 1)
    plot_last = Last_3;
  else if (title_idx == 2)
    plot_last = Last_6;
  else if (title_idx == 3)
    plot_last = Last_12;
  else if (title_idx == 4)
    plot_last = Last_24;
  else if (title_idx == 5)
    plot_last = Last_w;
  else if (title_idx == 6)
    plot_last = Last_m;

  if (title_idx >= 0){
    chart = CreateChart(data_plot, chart, plot_last);
    chart.render();
  }
}

function zeroPad(num, numZeros) {
  var n = Math.abs(num);
  var zeros = Math.max(0, numZeros - Math.floor(n).toString().length );
  var zeroString = Math.pow(10,zeros).toString().substr(1);
  if( num < 0 ) {
      zeroString = '-' + zeroString;
  }

  return zeroString+n;
}

function CreateChart(data_plot, chart, plot_last) {
  // var date_plot = [];
  var humid_data = [];
  var temp_data = [];
  var hid_data = [];
  var x_label = [];

  var min_temp = 100.0;
  var min_humid = 100.0;
  var max_temp = -100.0;
  var max_humid = -100.0;
  var curr_date = new Date();

  if (table.hasChildNodes()) {
    table.removeChild(table.childNodes[0]);
  }
  var tableBody = document.createElement('TBODY');
  table.appendChild(tableBody);

  var tr = document.createElement('TR');
  tableBody.appendChild(tr);

  var td = document.createElement('TH');
  td.width = '135';
  td.appendChild(document.createTextNode('Time Stamp'));
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
  td.width = '50';
  td.appendChild(document.createTextNode('HI (°C)'));
  tr.appendChild(td);

  for (var i = data_plot.length-1; i >= 0 ; i--) {
    var y1 = parseInt(data_plot[i][0].substring(0,4));
    var m1 = parseInt(data_plot[i][0].substring(4,6))-1;
    var d1 = parseInt(data_plot[i][0].substring(6,8));
    var H1 = parseInt(data_plot[i][0].substring(9,11));
    var Mi1 = parseInt(data_plot[i][0].substring(12,14));
    var S1 = parseInt(data_plot[i][0].substring(15,17));

    var new_date = new Date(y1, m1, d1,H1,Mi1,S1);

    // var HOUR24 = 24 * 60 * 60 * 1000;
    if ((curr_date - new_date) < plot_last){

      tr = null;
      tr = document.createElement('TR');
      tableBody.appendChild(tr);

      td = null;
      td = document.createElement('TD');
      td.width = '135';
      td.appendChild(document.createTextNode(CanvasJS.formatDate( new_date, "DD MMM YY HH:mm:ss")));
      tr.appendChild(td);

      td = null;
      td = document.createElement('TD');
      td.width = '45';
      td.appendChild(document.createTextNode(data_plot[i][4]));
      td.setAttribute("STYLE","background-color:" + data_plot[i][4].toLowerCase() + ";text-align: center;");
      if (data_plot[i][4].toLowerCase().localeCompare('black') === 0){
        td.setAttribute("STYLE","background-color:" + data_plot[i][4].toLowerCase() + ";color:white;text-align: center;");
      }
      tr.appendChild(td);

      td = null;
      td = document.createElement('TD');
      td.width = '50';
      td.appendChild(document.createTextNode(data_plot[i][2]));
      tr.appendChild(td);

      td = null;
      td = document.createElement('TD');
      td.width = '75';
      td.appendChild(document.createTextNode(data_plot[i][1]));
      tr.appendChild(td);

      td = null;
      td = document.createElement('TD');
      td.width = '50';
      td.appendChild(document.createTextNode(data_plot[i][3]));
      tr.appendChild(td);

      if (min_temp > parseFloat(data_plot[i][1]))
        min_temp = parseFloat(data_plot[i][1])

      if (min_temp > parseFloat(data_plot[i][3]))
        min_temp = parseFloat(data_plot[i][3])
      
      if (min_humid > parseFloat(data_plot[i][2]))
        min_humid = parseFloat(data_plot[i][2])

      if (max_temp < parseFloat(data_plot[i][1]))
        max_temp = parseFloat(data_plot[i][1])

      if (max_temp < parseFloat(data_plot[i][3]))
        max_temp = parseFloat(data_plot[i][3])
      
      if (max_humid < parseFloat(data_plot[i][2]))
        max_humid = parseFloat(data_plot[i][2])

      temp_data.push({x: new_date, y: data_plot[i][1], markerColor: data_plot[i][4].toLowerCase() });
      humid_data.push({x: new_date, y: data_plot[i][2], markerColor: data_plot[i][4].toLowerCase() });
      hid_data.push({x: new_date, y: data_plot[i][3], markerColor: data_plot[i][4].toLowerCase() });
    }
  }

  chart = new CanvasJS.Chart("chartContainer", {
    zoomEnabled: true,
    title:{
      text: "Humid & Temperature (" + chart_last_title[title_idx] + ')',
      fontSize: 20
    },	
    axisX:{
      valueFormatString: "DD MMM YY HH:mm:ss",

      labelFontSize: 12
    },
    axisY: [{
      minimum: parseInt(min_temp - 2),
      maximum: parseInt(max_temp + 2),
      title: "Degree Celsius (°C)",
      titleFontColor: "#4F81BC",
      lineColor: "#4F81BC",
      labelFontColor: "#4F81BC",
      tickColor: "#4F81BC"
    },
    {
      minimum: parseInt(min_temp - 2),
      maximum: parseInt(max_temp + 2),
      title: "Heat Index (°C)",
      titleFontColor: "#67963f",
      lineColor: "#67963f",
      labelFontColor: "#67963f",
      tickColor: "#67963f"
    }],
    axisY2: {
      minimum: parseInt(min_humid - 2),
      maximum: parseInt(max_humid + 2),
      title: "Humid (%)",
      titleFontColor: "#C0504E",
      lineColor: "#C0504E",
      labelFontColor: "#C0504E",
      tickColor: "#C0504E",
    },	
    toolTip: {
      shared: true,
      contentFormatter: function (e) {
        var content = " ";
        content += CanvasJS.formatDate( e.entries[0].dataPoint.x, "DD MMM YY HH:mm:ss");
				for (var i = 0; i < e.entries.length; i++) {
          content += "<br/>";
					content += e.entries[i].dataSeries.name + " " + "<strong>" + e.entries[i].dataPoint.y + "</strong>";	
        }
        content += "<p id='xData' style='display:none;'>"+ e.entries[0].dataPoint.x+ "</p>";
				return content;
			}
    },
    legend: {
      cursor:"pointer",
      itemclick: toggleDataSeries
    },
    data: [{
      type: "line",
      markerType: "circle",
      markerBorderColor: "black",
      markerBorderThickness: 1,
      markerSize: 7,
      name: "Temperature (°C)",
      legendText: "Temperature",
      axisYIndex: 0,
      showInLegend: true, 
      dataPoints: temp_data
    },
    {
      type: "line",	
      markerType: "circle",
      markerBorderColor: "black",
      markerBorderThickness: 1,
      markerSize: 7,
      name: "Humid (%)",
      legendText: "Humid",
      axisYType: "secondary",
      showInLegend: true,
      dataPoints: humid_data
    },
    {
      type: "line",	
      markerType: "circle",
      markerBorderColor: "black",
      markerBorderThickness: 1,
      markerSize: 7,
      name: "Heat Index (°C)",
      legendText: "Heat Index (°C)",
      axisYIndex: 1,
      showInLegend: true,
      dataPoints: hid_data
    }]
  });

  myTableDiv.appendChild(table);
  return chart;
}

ref.on('value',function (snapshot) {
  chart.destroy();
  data_plot = [];

  snapshot.forEach(function(childSnapshot) {       
    var keytime = childSnapshot.key;     
    
    var db_id = database.ref('/ID').child(key + '/Log/' + keytime);
    db_id.on('value',function (snapshot_id) {
      var data = snapshot_id.val()

      var data_subplot = [data['DateTime'], data['Temperature'], data['Humid'], data['HID'], data['Flag']];
      data_plot.push(data_subplot);
    });
  });

  chart = CreateChart(data_plot, chart, plot_last);
  chart.render();
  
});