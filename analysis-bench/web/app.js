'use strict';
const READER_VERSION = '0.6.0';
const $ = id => document.getElementById(id);
const state = {run:null, name:null, view:'count', page:0, selected:null, from:0, to:Infinity, token:0, rows:[], filtered:[], validSettings:{ppr:1,window:1,ratio:3,floor:0}};
const fmt = (n, places=3) => n === null || n === undefined || !Number.isFinite(n) ? '—' : n.toLocaleString('en-GB',{maximumFractionDigits:places});
const median = values => { const sorted=[...values].sort((a,b)=>a-b); const n=sorted.length; return n ? (sorted[Math.floor((n-1)/2)]+sorted[Math.floor(n/2)])/2 : null; };
function settings() {
  const ppr=Number($('ppr').value), window=Number($('window').value), ratio=Number($('ratio').value), floor=Number($('floor').value);
  if (!$('settings').checkValidity() || !Number.isInteger(ppr) || !Number.isInteger(window) || window%2!==1) throw Error('Use valid settings: detections/revolution 1–100, an odd window 1–101, ratio 1.1–100 and a nonnegative short-interval threshold.');
  return {ppr,window,ratio,floor};
}
async function get(url) { const response=await fetch(url); const result=await response.json(); if (!response.ok) throw Error(result.error || 'Could not read recording.'); return result; }
async function refresh() {
  $('error').textContent=''; $('refresh').disabled=true;
  try {
    const {runs}=await get('/api/runs'); $('runCount').textContent=String(runs.length); $('runs').replaceChildren();
    for (const run of runs) {
      const button=document.createElement('button'); button.className='run'; button.dataset.name=run.name; button.title=run.name;
      const name=document.createElement('span'); name.className='run-name'; name.textContent=run.name;
      const size=document.createElement('span'); size.className='run-size'; size.textContent=run.size ? `${fmt(run.size,0)} bytes · optical event file` : 'Empty recording · 0 bytes';
      button.append(name,size); button.addEventListener('click',()=>openRun(run.name)); $('runs').append(button);
    }
    if (!runs.length) { $('runs').textContent='No BIN recordings found. Copy files into the runs folder, then refresh.'; clearRun(); }
    else if (state.name && runs.some(r=>r.name===state.name)) await openRun(state.name);
    else await openRun([...runs].sort((a,b)=>b.size-a.size)[0].name);
  } catch(error) { $('error').textContent=error.message; }
  finally { $('refresh').disabled=false; }
}
function endSeconds(events) { let end=0; for(const event of events) end=Math.max(end,event[1]/1e6); return end; }
function clearRun() { state.run=null; state.name=null; $('reader').hidden=true; $('empty').hidden=false; }
async function openRun(name, references=false) {
  const token=++state.token;
  $('error').textContent=''; $('referenceResult').textContent='';
  try {
    const query=new URLSearchParams({name});
    if (references) {
      if ($('expectedCount').value!=='') query.set('expected_events',$('expectedCount').value);
      if ($('expectedCrc').value.trim()) query.set('expected_crc',$('expectedCrc').value.trim());
      if (!query.has('expected_events') && !query.has('expected_crc')) throw Error('Enter the expected count and CRC from the Serial summary.');
    }
    const run=await get('/api/run?'+query);
    if (token!==state.token) return;
    const changed=state.name!==name;
    state.run=run; state.references={count:query.get('expected_events'),crc:query.get('expected_crc')}; state.name=name; state.page=0;
    if (!references || changed) {
      state.selected=null; state.from=0; state.to=Infinity;
      $('from').value='0'; $('to').value=String(endSeconds(run.events));
      $('expectedCount').value=''; $('expectedCrc').value='';
    }
    $('empty').hidden=true; $('reader').hidden=false; $('filename').textContent=name;
    document.querySelectorAll('.run').forEach(b=>{b.classList.toggle('active',b.dataset.name===name);b.setAttribute('aria-pressed',String(b.dataset.name===name));});
    const validation=run.validation;
    $('status').className='status'+(validation.structure==='fail'?' fail':validation.completion!=='reference-match'?' warning':'');
    $('status').textContent=validation.message;
    $('timeNote').textContent='Recording date unknown · elapsed time since test start · optical events v1';
    $('fileInfo').textContent=`${fmt(run.size,0)} bytes · CRC32 ${run.crc32} · Structure ${validation.structure.toUpperCase()} · Reference ${validation.completion}`;
    $('hash').textContent='Source SHA-256: '+run.sha256;
    if (references) $('referenceResult').textContent=validation.message;
    render();
  } catch(error) {
    if (token!==state.token) return;
    if (references) $('referenceResult').textContent=error.message;
    else { clearRun(); $('error').textContent=error.message; }
  }
}
function render() {
  if (!state.run) return;
  let vars;
  try { vars=settings(); state.validSettings=vars; $('settingsError').textContent=''; }
  catch(error) { vars=state.validSettings; $('settingsError').textContent=error.message+' Showing this recording with the last valid analysis settings until corrected.'; }
  const run=state.run;
  const intervals=run.events.slice(1).map((event,i)=>event[1]-run.events[i][1]);
  const baseline=median(intervals.filter(i=>i>0));
  const raw=run.events.map((event,i)=>i && intervals[i-1]>0 ? 60e6/intervals[i-1]/vars.ppr : null);
  const half=Math.floor(vars.window/2);
  state.rows=run.events.map(([sequence,time],i)=>{
    const interval=i ? intervals[i-1] : null, reasons=[];
    if (sequence!==i+1) reasons.push('sequence');
    if (interval!==null && interval<=0) reasons.push('time order');
    else if (interval!==null && baseline!==null) {
      if (interval<baseline/vars.ratio) reasons.push('fast interval');
      if (interval>baseline*vars.ratio) reasons.push('slow interval');
      if (interval<vars.floor*1000) reasons.push('short interval');
    }
    const smooth=raw[i]===null ? null : vars.window===1 ? raw[i] : median(raw.slice(Math.max(0,i-half),Math.min(raw.length,i+half+1)).filter(v=>v!==null));
    return {index:i,sequence,time,interval,rpm:raw[i],smooth,reasons,revs:(i+1)/vars.ppr};
  });
  const inside=state.rows.filter(r=>r.time/1e6>=state.from && r.time/1e6<=state.to);
  state.filtered=inside.filter(r=>!$('flaggedOnly').checked || r.reasons.length);
  $('revolutions').textContent=fmt(run.event_count/vars.ppr,2); $('detections').textContent=fmt(run.event_count,0);
  $('span').textContent=run.validation.structure==='fail'||run.stats.span_us===null ? '—' : fmt(run.stats.span_us/1e6)+' s';
  $('rpm').textContent=fmt(run.stats.mean_rpm===null ? null : run.stats.mean_rpm/vars.ppr,1);
  $('flags').textContent=fmt(state.rows.filter(r=>r.reasons.length).length,0);
  draw(inside,vars); renderTable(); showSelection();
}
function plotRows(rows) {
  if (rows.length<=2000) return rows;
  const bucket=Math.ceil(rows.length/700), selected=new Set([0,rows.length-1]);
  for(let start=0;start<rows.length;start+=bucket) {
    let min=start,max=start;
    for(let i=start;i<Math.min(rows.length,start+bucket);i++) {
      if ((rows[i].rpm??0)<(rows[min].rpm??0)) min=i;
      if ((rows[i].rpm??0)>(rows[max].rpm??0)) max=i;
    }
    selected.add(start);selected.add(min);selected.add(max);
  }
  return [...selected].sort((a,b)=>a-b).map(i=>rows[i]);
}
function draw(rows,vars) {
  $('chartTitle').textContent=state.view==='count'?'Detected revolutions over time':'Revolution speed from event intervals';
  const display=plotRows(rows), countView=state.view==='count';
  $('chartNote').textContent=display.length<rows.length ? `${fmt(display.length,0)} plotted / ${fmt(rows.length,0)} events; table retains all` : `${fmt(rows.length,0)} events in range`;
  if (!rows.length) { $('chart').innerHTML='<text x="500" y="155" text-anchor="middle">No events in this range.</text>'; return; }
  let minX=state.from, maxX=Number.isFinite(state.to)?state.to:0;
  if(!Number.isFinite(state.to)) for(const row of rows) maxX=Math.max(maxX,row.time/1e6);
  if (maxX<=minX) maxX=minX+1;
  let maxY=0;
  for(const r of rows) maxY=Math.max(maxY,(countView?r.revs:Math.max(r.rpm??0,r.smooth??0)));
  maxY=maxY>0?maxY*1.08:1;
  const x=t=>65+(t/1e6-minX)/(maxX-minX)*910, y=v=>265-v/maxY*228;
  let svg='';
  for(let i=0;i<=4;i++) {
    const yy=265-i*57,xx=65+i*227.5;
    svg+=`<line x1="65" y1="${yy}" x2="975" y2="${yy}" class="grid-line"/><text x="53" y="${yy+4}" text-anchor="end">${fmt(maxY*i/4,1)}</text>`;
    svg+=`<text x="${xx}" y="286" text-anchor="middle">${fmt(minX+(maxX-minX)*i/4,2)}</text>`;
  }
  svg+=`<text x="65" y="20">${countView?'Revolutions (assumed)':'RPM (assumed)'}</text><text x="520" y="310" text-anchor="middle">Elapsed since test start (seconds)</text>`;
  function path(field,step) {
    let result='',begun=false;
    for(const row of display) {
      const value=row[field];
      if(value===null) {begun=false;continue;}
      if(!begun) {result+=`M${x(row.time)},${y(value)}`;begun=true;}
      else result+=step?`H${x(row.time)}V${y(value)}`:`L${x(row.time)},${y(value)}`;
    }
    return result;
  }
  svg+=`<path d="${path(countView?'revs':'rpm',countView)}" class="raw-line"/>`;
  if(!countView && vars.window>1) svg+=`<path d="${path('smooth',false)}" class="smooth-line"/>`;
  for(const row of display) {
    const value=countView?row.revs:row.rpm;
    if(value===null)continue;
    const cls=row.index===state.selected?'selected-dot':row.reasons.length?'review-dot':'event-dot';
    svg+=`<circle cx="${x(row.time)}" cy="${y(value)}" r="${row.index===state.selected?5:3}" class="${cls}" data-index="${row.index}"><title>Event ${row.index+1}, ${fmt(row.time/1e6,6)} s, ${fmt(row.rpm,2)} RPM</title></circle>`;
  }
  $('chart').innerHTML=svg;
  $('chart').querySelectorAll('[data-index]').forEach(dot=>dot.addEventListener('click',()=>selectRow(Number(dot.dataset.index))));
}
function renderTable() {
  const count=state.filtered.length, pages=Math.max(1,Math.ceil(count/100)); state.page=Math.min(state.page,pages-1);
  $('events').replaceChildren();
  for(const row of state.filtered.slice(state.page*100,(state.page+1)*100)) {
    const tr=document.createElement('tr');tr.dataset.index=String(row.index);tr.classList.toggle('chosen',row.index===state.selected);
    const values=[row.index+1,row.sequence,fmt(row.time/1e6,6),fmt(row.interval===null?null:row.interval/1000,3),fmt(row.rpm,2),fmt(row.smooth,2),row.reasons.join(' · ')||'—'];
    for(const value of values) {const td=document.createElement('td');td.textContent=String(value);tr.append(td);}
    tr.lastChild.className='flag-tag';tr.addEventListener('click',()=>selectRow(row.index));
    const button=document.createElement('button');button.className='text-button';button.textContent=String(row.index+1);button.setAttribute('aria-label',`Inspect event ${row.index+1}`);button.addEventListener('click',event=>{event.stopPropagation();selectRow(row.index);});tr.firstChild.replaceChildren(button);
    $('events').append(tr);
  }
  if(!count) {const tr=document.createElement('tr'),td=document.createElement('td');td.colSpan=7;td.textContent='No events match this range/filter.';tr.append(td);$('events').append(tr);}
  $('pageInfo').textContent=`${count?state.page*100+1:0}–${Math.min(count,(state.page+1)*100)} of ${fmt(count,0)} events`;
  $('previous').disabled=state.page===0;$('next').disabled=state.page+1>=pages;
}
function selectRow(index) {
  state.selected=index;
  const position=state.filtered.findIndex(r=>r.index===index);if(position>=0)state.page=Math.floor(position/100);
  render();
  $('events').querySelector(`tr[data-index="${index}"] button`)?.focus({preventScroll:true});
}
function showSelection() {
  const row=state.rows[state.selected];
  $('selection').textContent=row ? `Event ${row.index+1} · raw sequence ${row.sequence} · ${fmt(row.time/1e6,6)} s · interval ${fmt(row.interval===null?null:row.interval/1000,3)} ms · ${fmt(row.rpm,2)} RPM · ${row.reasons.join(', ')||'No review flag'}` : 'Select a plotted event or table row to inspect it.';
}
function applyRange() {
  const from=Number($('from').value),to=Number($('to').value);
  if(!Number.isFinite(from)||!Number.isFinite(to)||from<0||to<from) {$('settingsError').textContent='Display range must have a nonnegative start and an end at or after it.';return;}
  state.from=from;state.to=to;state.page=0;render();
}
function exportCsv() {
  if(!state.run)return;
  try {settings();}catch(error){$('error').textContent=error.message;return;}
  const vars=settings();
  const quote=value=>'"'+String(value??'').replaceAll('"','""')+'"';
  const header=['reader_version','file','sha256','source_crc32','reference_count','reference_crc32','structure','completion','detections_per_revolution','rpm_median_window','interval_ratio','short_interval_ms','range_from_s','range_to_s','raw_event','raw_sequence','elapsed_us','interval_us','raw_rpm','median_rpm','review_flags'];
  const rows=state.rows.filter(r=>r.time/1e6>=state.from&&r.time/1e6<=state.to).map(r=>[READER_VERSION,state.run.name,state.run.sha256,state.run.crc32,state.references?.count,state.references?.crc,state.run.validation.structure,state.run.validation.completion,vars.ppr,vars.window,vars.ratio,vars.floor,state.from,Number.isFinite(state.to)?state.to:'',r.index+1,r.sequence,r.time,r.interval,r.rpm,r.smooth,r.reasons.join(';')].map(quote).join(','));
  const blob=new Blob(['\uFEFF'+header.join(',')+'\r\n'+rows.join('\r\n')],{type:'text/csv;charset=utf-8'});
  const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=state.name.split('/').pop().replace(/\.bin$/i,'')+'-events.csv';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000);
}
$('settings').addEventListener('submit',event=>event.preventDefault());
$('settings').addEventListener('input',()=>{state.page=0;render();});
$('refresh').addEventListener('click',refresh);
$('reset').addEventListener('click',()=>{$('ppr').value='1';$('window').value='1';$('ratio').value='3';$('floor').value='0';state.page=0;render();});
for(const [id,view] of [['countView','count'],['rpmView','rpm']])$(id).addEventListener('click',()=>{state.view=view;for(const [button,value] of [['countView','count'],['rpmView','rpm']]){$(button).classList.toggle('selected',value===view);$(button).setAttribute('aria-pressed',String(value===view));}render();});
$('applyRange').addEventListener('click',applyRange);
$('fullRange').addEventListener('click',()=>{state.from=0;state.to=Infinity;$('from').value='0';$('to').value=String(endSeconds(state.run?.events||[]));state.page=0;render();});
$('flaggedOnly').addEventListener('change',()=>{state.page=0;render();});
$('previous').addEventListener('click',()=>{state.page--;renderTable();});$('next').addEventListener('click',()=>{state.page++;renderTable();});
$('checkReference').addEventListener('click',()=>openRun(state.name,true));
$('export').addEventListener('click',exportCsv);
refresh();
