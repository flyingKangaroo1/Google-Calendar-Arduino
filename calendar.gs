/*
  Based on code by  Seweryn Talaj  github.com/Seweryn91
  https://github.com/Seweryn91/Google-Calendar-with-Node-MCU
*/


function doGet(e) {
  var calendarName = '[calendar 이름 입력]'

  var cal = CalendarApp.getCalendarsByName(calendarName)[0];
  if (cal == undefined) {
    return ContentService.createTextOutput('No access to calendar! ');}
  else{
    if(checkForEventsToday(cal) == true){
      var retrievedEvents = getEvents(cal);
      // if there are no current events, display events later
      if(retrievedEvents == ''){
        return ContentService.createTextOutput('Events later: \n' + getAllEventsHappeningLater(cal));}
      else{
        return ContentService.createTextOutput('Current events: \n' + getEvents(cal));}
    }
    else{
      return ContentService.createTextOutput('No events scheduled \n');}
  }
 }

function checkForEventsToday(cal){
  var now = new Date();
  var dayEnd = new Date();
  dayEnd.setHours(23,59,59);
  var events = cal.getEvents(now, dayEnd);
  
  if(events.length > 0){
      return true;
  }else{
    return false;
 }
}

function areEventsHappeningNow(cal){
  var now = new Date();
  var later = new Date();
  later.setMinutes(now.getMinutes() + 1);
  var events = cal.getEvents(now, later);
  
  if(events.length > 0){
    areEventsHappening = true;}
  else{ 
    return false;
      }
}

function getAllEventsHappeningLater(cal){
  var now = new Date();
  var dayEnd = new Date();
  dayEnd.setHours(23,59,59);
  var events = cal.getEvents(now, dayEnd);

  var str = '';
    
  for (var ii = 0; ii < events.length; ii++) {
    var event=events[ii]; 
    str += event.getTitle() +'\n'+
      formatDate(event.getStartTime()) + ' to '+
      formatDate(event.getEndTime()) +'\n';
    }
  return str;
}


function getEvents(cal){
  var now = new Date();
  var later = new Date();
  later.setMinutes(now.getMinutes() + 1);

  var events = cal.getEvents(now, later);
  var str = '';
    
  for (var ii = 0; ii < events.length; ii++) {
    var event=events[ii]; 
    str += event.getTitle() +'\n'+
      formatDate(event.getStartTime()) + ' to '+
      formatDate(event.getEndTime()) +'\n';
    }
  return str;
}

function formatDate(date){
  var formattedDate = Utilities.formatDate(date, "GMT+1", "HH:mm");
  return formattedDate;
}
