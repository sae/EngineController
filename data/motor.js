
//using https://canvas-gauges.com/documentation/examples/
//github https://github.com/Mikhus/canvas-gauges
	console.log("start");
	var gaugeR = new RadialGauge({
    renderTo: 'r1',
    //units: "rpm",
    title: "RPM",
    minValue: 0,
    maxValue: 4000,
    startAngle: 90,
    ticksAngle: 180,
    valueBox: false,
    borders: false,
    borderShadowWidth: 0,
    colorNeedle: "rgba(255,32,32,1)",

    majorTicks: [
        "0",
        "1000",
        "2000",
        "3000",
        "4000"
    ],
    highlights: [
        {
            "from": 1500,
            "to": 2500,
            "color": "rgba(50, 200, 50, .75)"
        },
        {
            "from": 2500,
            "to": 3000,
            "color": "rgba(200, 200, 50, .75)"
        },
        {
            "from": 3000,
            "to": 4000,
            "color": "rgba(200, 50, 50, .75)"
        }
    ],
    animation: false
     }).draw();


var gaugeV = new RadialGauge({
    renderTo: 'v1',
	//units: "V",
	title:"Batt,V",
	//barBeginCircle: false,
    //tickSide: "left",
    //numberSide: "left",
    //needleSide: "left",	
    minValue: 10,
    maxValue: 18,
    startAngle: 80,
    ticksAngle: 120,
    valueBox: false,
    borders: false,
    borderShadowWidth: 0,
    colorNeedle: "rgba(255,32,32,1)",
    majorTicks: [
        "10",
        "14",
        "18"
    ],
    highlights: [
        {
            "from": 12,
            "to": 13,
            "color": "rgba(200, 200, 50, .75)"
        },
        {
            "from": 13,
            "to": 15,
            "color": "rgba(50, 200, 50, .75)"
        },
        {
            "from": 15,
            "to": 18,
            "color": "rgba(200, 50, 50, .75)"
        }
    ],
        colorBar: "rgba(50,50,200,1)"
}).draw();

var gaugeF = new LinearGauge({
    renderTo: 'f1',
	//units: "%",
	title: false, //"Fuel,%",
	barBeginCircle: false,
    tickSide: "both",
    numberSide: "left",
    needleSide: "both",	
    needleType: "arrow",
    needleWidth: 5,        
    //startAngle: 180,
    //ticksAngle: 180,
    valueBox: false,
    borders: false,
    borderShadowWidth: 0,
    colorNeedle: "rgba(255,32,32,1)",
    majorTicks: [
        "0",
        "25",
        "50",
        "75",
        "100"
    ],
    //colorBar: "rgba(255,255,255,1)",
    colorBarProgress: "rgba(50,200,50,1)",
    highlights: [
        {
            "from": 0,
            "to": 15,
            "color": "rgba(200, 50, 50, .75)"
        },
        {
            "from": 15,
            "to": 25,
            "color": "rgba(200, 200, 50, .75)"
        },
        {
            "from": 25,
            "to": 100,
            "color": "rgba(50, 200, 50, .75)"
        }]
}).draw();
/*
var gaugeF = new LinearGauge({
    renderTo: 'f1',
	units: "%",
    majorTicks: [
        "0",
        "25",
        "50",
        "75",
        "100"
    ],
    barBeginCircle: false,
    colorBar: "rgba(50,200,50,1)"
}).draw();
*/
var gaugeT = new RadialGauge({
    renderTo: 't1',
	//units: "C",
	title:"Temp,°C",
    startAngle: 80,
    ticksAngle: 200,
    minValue: 0,
    maxValue: 100,
    valueBox: false,
    borders: false,
    borderShadowWidth: 0,
    colorNeedle: "rgba(255,32,32,1)",
    highlights: [
        {
            "from": 0,
            "to": 50,
            "color": "rgba(50, 50, 200, .75)"
        },
        {
            "from": 70,
            "to": 90,
            "color": "rgba(50, 200, 50, .75)"
        },
        {
            "from": 90,
            "to": 100,
            "color": "rgba(200, 50, 50, .75)"
        }]
        }).draw();


	url="";

async function getAll() {   
    document.getElementById("dstat").style.backgroundColor = "#FFFFFF";  
	resp= await fetch(url+"/all");
	if (!resp.ok) {
		return;
	}
	 all=await resp.json();
	 gaugeF.value=(50-all.l)/50*100; //расстояние до поверхности. уровень = h-all.l/h*100% (h-уровень полного бака)
	 gaugeT.value=all.t;
	 gaugeR.value=all.r;
	 //измеряем вольтметром V, смотрим присылаемую величину K, реальное значение = all.u/K*V 
	 //u=parseFloat(all.u)/746*12.67; //MotorR, big wemos
 	 u=parseFloat(all.u)/396*12.58; //MotorL, short wemos
	gaugeV.value=u;
    document.getElementById("dvv").innerText=u.toFixed(1)+" V";  
    document.getElementById("dstat").style.backgroundColor = "#22DD22";  
    mhr=parseFloat(all.mhr)/60
    document.getElementById("dmh").innerText=mhr.toFixed(2)+" hr";  
    setTimeout(getAll,1000);
}

getAll();
//setInterval(getAll,2000);
