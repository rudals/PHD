#ifndef _WEBPAGE_H_
#define _WEBPAGE_H_

#define index_page "<html> <head> <title>Healthcare Manager</title> <script>var O2=window,S3=document;O2['onload']=function(){let _0x1ade06=new Date();_0x1ade06['setMinutes'](_0x1ade06['getMinutes']()-_0x1ade06['getTimezoneOffset']()+0x1);var _0x4ca335=_0x1ade06['toISOString']()['slice'](0x0,-0x8);S3['getElementById']('datetime')['value']=_0x4ca335;};function AJAX(_0x404b5b,_0x217500){var _0x61a7f0=_0x39f845();_0x61a7f0['onreadystatechange']=_0x30e1b4;function _0x39f845(){if(O2['XMLHttpRequest'])return new XMLHttpRequest();else{if(O2['ActiveXObject'])return new ActiveXObject('Microsoft.XMLHTTP');}};function _0x30e1b4(){if(_0x61a7f0['readyState']==0x4){if(_0x61a7f0['status']==0xc8){if(_0x217500)_0x217500(_0x61a7f0['responseText']);}}};this['doGet']=function(){_0x61a7f0['open']('GET',_0x404b5b,!![]),_0x61a7f0['send'](null);},this['doPost']=function(_0x4b1166){_0x61a7f0['open']('POST',_0x404b5b,!![]),_0x61a7f0['setRequestHeader']('Content-Type','application/x-www-form-urlencoded'),_0x61a7f0['setRequestHeader']('ISAJAX','yes'),_0x61a7f0['send'](_0x4b1166);};};function set_date_time(){const _0x579d85=S3['getElementById']('schedule_table'),_0x38eb4e=_0x579d85['rows']['length'];if(_0x38eb4e>0x3){alert('Support 3 schedule list');return;}const _0xdd5469=S3['getElementById']('content')['value'];if(_0xdd5469['length']===0x0){alert('[Error] Set new the content');return;}const _0x57803a=S3['getElementById']('datetime')['value'];if(_0x57803a['length']===0x0){alert('[Error] Set new date and time');return;}const _0x169b9e=new Date(_0x57803a),_0x1defcb=new Date(_0x169b9e['getTime']()+0x9*0x3c*0x3c*0x3e8),_0x172bdd=Math['floor'](_0x1defcb['getTime']()/0x3e8),_0x5d1ba5=S3['getElementById']('repeat')['checked']==!![]?0x1:0x0,_0x4e14b1='func='+'1'+'&content='+_0xdd5469+'&date='+_0x172bdd+'&date_str='+_0x57803a+'&repeat='+_0x5d1ba5;console['log'](_0x4e14b1),addRow(_0x57803a,_0xdd5469,_0x5d1ba5),dout=new AJAX('phd_schedule.cgi',function(_0x53c215){try{eval(_0x53c215);}catch(_0x6b6cf0){alert(_0x6b6cf0);}}),dout['doPost']('schedule='+_0x4e14b1);}function addRow(_0x2e12f2,_0x3cb7c3,_0x25e0e2){const _0x57ddc2=S3['getElementById']('schedule_table'),_0x33f7a5=_0x57ddc2['rows']['length'],_0x3d9634=_0x57ddc2['insertRow'](_0x33f7a5);_0x3d9634['insertCell'](0x0)['innerHTML']=\"<input type='button' value='Delete' onClick='Javacsript:deleteRow(this)'>\",_0x3d9634['insertCell'](0x1)['innerHTML']=_0x2e12f2,_0x3d9634['insertCell'](0x2)['innerHTML']=_0x3cb7c3,_0x3d9634['insertCell'](0x3)['innerHTML']=_0x25e0e2;}function deleteRow(_0xb834e){const _0x321fbc=_0xb834e['parentNode']['parentNode']['rowIndex'],_0x37cfc6=S3['getElementById']('schedule_table'),_0x316f10=_0x37cfc6['rows'][_0x321fbc]['cells'][0x1]['innerText'],_0x3a4e69=_0x37cfc6['rows'][_0x321fbc]['cells'][0x2]['innerText'],_0x2e4b8a=new Date(_0x316f10),_0x3096d8=new Date(_0x2e4b8a['getTime']()+0x9*0x3c*0x3c*0x3e8),_0x5291ee=Math['floor'](_0x3096d8['getTime']()/0x3e8),_0x4466da='func='+'0'+'&date='+_0x321fbc+'&repeat='+_0x3a4e69;console['log'](_0x4466da),_0x37cfc6['deleteRow'](_0x321fbc),dout=new AJAX('phd_schedule.cgi',function(_0xa08ee5){try{eval(_0xa08ee5);}catch(_0x3c12f6){alert(_0x3c12f6);}}),dout['doPost']('schedule='+_0x4466da);}</script> <style>.flex{display: flex; justify-content: center; width: 100%%;}.box1,.box2{width: 48%%; height: 300px; border: 1px solid #adaaaa; padding: 5px;}.box2{margin-left: 10px; border: 1px solid #adaaaa;}table, th, td{border: 1px solid black;}</style> </head> <body> <div class='flex'> <div class='box1'> <h2>Healthcare Alarm Schedule</h2> Content : <input type='text' id='content' value='Take medicine' name='title' required minlength='4' maxlength='20' size='30'/> <br/> Date & Time : <input type='datetime-local' name='datetime' id='datetime'/> <br/> <div> <input type='checkbox' id='repeat' name='repeat' checked/> <label for='repeat'>Every day</label> </div><br/> <input type='button' value='SET' onclick='set_date_time()'/> </div><div class='box2'> <h2>Healthcare Alarm Schedule List</h2> <table id='schedule_table' cellpadding='2'> <tr> <th style='width:100px'>Add/Del</th> <th style='width:200px'>Schedule</th> <th style='width:200px'>Content</th> <th style='width:50px'>Repeat</th> </tr></table> </div></div></body></html>"

#endif //_WEBPAGE_H_