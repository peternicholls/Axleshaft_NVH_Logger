'use strict';
const READER_VERSION = '0.6.1';
const $ = id => document.getElementById(id);
const state = {run:null, name:null, view:'rpm', page:0, selected:null, from:0, to:Infinity, token:0, rows:[], filtered:[], validSettings:{ppr:1,window:1,ratio:3,floor:0}, runs:[]};
const fmt = (n, places=3) => n === null || n === undefined || !Number.isFinite(n) ? '—' : n.toLocaleString('en-GB',{maximumFractionDigits:places});
const median = values => { const sorted=[...values].sort((a,b)=>a-b); const n=sorted.length; return n ? (sorted[Math.floor((n-1)/2)]+sorted[Math.floor(n/2)])/2 : null; };
function settings() {
  const ppr=Number($('ppr').value), window=Number($('window').value), ratio=Number($('ratio').value), floor=Number($('floor').value);
  if (!$('settings').checkValidity() || !Number.isInteger(ppr) || !Number.isInteger(window) || window%2!==1) throw Error('Use valid settings: detections/revolution 1–100, an odd window 1–101, ratio 1.1–100 and a nonnegative short-interval threshold.');
  return {ppr,window,ratio,floor};
}
async function get(url) { const response=await fetch(url); const result=await response.json(); if (!response.ok) throw Error(result.error || 'Could not read recording.'); return result; }
async function refresh(preferred=state.name) {
  $('error').textContent=''; $('refresh').disabled=true;
  try {
    const {runs}=await get('/api/runs'); state.runs=runs; $('runCount').textContent=String(runs.length); paintLibrary();
    const visible=visibleRuns();
    if (!visible.length) { clearRun(); }
    else if (preferred && visible.some(r=>r.name===preferred)) await openRun(preferred);
    else await openRun([...visible].sort((a,b)=>b.size-a.size)[0].name);
  } catch(error) { $('error').textContent=error.message; }
  finally { $('refresh').disabled=false; }
}
function visibleRuns() { return state.runs.filter(r=>$('showHidden').checked || !r.metadata?.hidden); }
function paintLibrary() {
  $('runs').replaceChildren();const visible=visibleRuns();
  for (const run of visible) {
    const button=document.createElement('button'); button.className='run'; button.dataset.name=run.name;
    button.title=run.name;button.classList.toggle('active',run.name===state.name);button.setAttribute('aria-pressed',String(run.name===state.name));button.classList.toggle('marked',!!run.metadata?.marked);button.classList.toggle('hidden-run',!!run.metadata?.hidden);
    const name=document.createElement('span');name.className='run-name';name.textContent=run.metadata?.label||run.name;
    const size=document.createElement('span');size.className='run-size';size.textContent=run.size?`${fmt(run.size,0)} bytes · optical event file`:'Empty recording · 0 bytes';
    button.append(name,size);button.addEventListener('click',()=>openRun(run.name));$('runs').append(button);
  }
  if(!visible.length) $('runs').textContent=state.runs.length?'All recordings are hidden. Enable Show hidden recordings to restore them.':'No BIN recordings found. Copy files into the runs folder, then refresh.';
}
async function saveMetadata(patch) {
  if(!state.run)return;
  const name=state.name; $('recordActionStatus').textContent='Saving…';
  ['saveLabel','renameRun','markRun','hideRun'].forEach(id=>$(id).disabled=true);
  try {
    const response=await fetch('/api/metadata',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name,...patch})});
    const body=await response.json();if(!response.ok)throw Error(body.error||'Could not save recording label.');
    if(state.name===name){state.run.metadata=body.metadata;updateIdentity();$('renamePanel').hidden=true;}
    const listed=state.runs.find(r=>r.name===name);if(listed)listed.metadata=body.metadata;
    paintLibrary();
    if(body.metadata.hidden&&!$('showHidden').checked&&state.name===name) {
      const next=visibleRuns()[0];if(next)await openRun(next.name);else clearRun();
    }
    $('recordActionStatus').textContent='Saved';
  }catch(error){$('recordActionStatus').textContent=error.message;}
  finally{['saveLabel','renameRun','markRun','hideRun'].forEach(id=>$(id).disabled=false);}
}
function updateIdentity() {
  const run=state.run;if(!run)return;
  $('filename').textContent=run.metadata?.label||run.name;
  $('markRun').textContent=run.metadata?.marked?'Unmark recording':'Mark recording';
  $('markRun').setAttribute('aria-pressed',String(!!run.metadata?.marked));
  $('hideRun').textContent=run.metadata?.hidden?'Restore recording':'Hide recording';
  $('fileInfo').textContent=`Original: ${run.name} · ${fmt(run.size,0)} bytes · CRC32 ${run.crc32} · Structure ${run.validation.structure.toUpperCase()} · Reference ${run.validation.completion}`;
  document.querySelectorAll('.run').forEach(button=>{button.classList.toggle('active',button.dataset.name===run.name);button.setAttribute('aria-pressed',String(button.dataset.name===run.name));});
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
    $('empty').hidden=true; $('reader').hidden=false; $('renamePanel').hidden=true; $('recordActionStatus').textContent='';
    document.querySelectorAll('.run').forEach(b=>{b.classList.toggle('active',b.dataset.name===name);b.setAttribute('aria-pressed',String(b.dataset.name===name));});
    const validation=run.validation;
    $('status').className='status'+(validation.structure==='fail'?' fail':validation.completion!=='reference-match'?' warning':'');
    $('status').textContent=validation.message;
    $('timeNote').textContent='Recording date unknown · elapsed time since test start · optical events v1';
    $('fileInfo').textContent=`${fmt(run.size,0)} bytes · CRC32 ${run.crc32} · Structure ${validation.structure.toUpperCase()} · Reference ${validation.completion}`;
    $('hash').textContent='Source SHA-256: '+run.sha256;updateIdentity();
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
  draw(inside,vars); renderOverview(); renderFocusStats(); renderTable(); showSelection();
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
function focusEnd() { return Math.round(endSeconds(state.run?.events||[])*1e6); }
function setFocus(left,right) {
  const bounds=RotationMath.boundedRange(left,right,focusEnd());
  state.from=bounds.from/1e6;state.to=bounds.to/1e6;state.page=0;
  $('from').value=String(state.from);$('to').value=String(state.to);render();
}
function applyRange() {
  const from=Number($('from').value),to=Number($('to').value),end=focusEnd()/1e6;
  if(!Number.isFinite(from)||!Number.isFinite(to)||from<0||to<from||to>end) {
    $('settingsError').textContent=`Display range must lie between 0 and ${fmt(end,6)} seconds, with end at or after start.`;return;
  }
  setFocus(Math.round(from*1e6),Math.round(to*1e6));
}
function renderFocusStats() {
  const left=Math.round(state.from*1e6),right=Number.isFinite(state.to)?Math.round(state.to*1e6):focusEnd();
  const valid=state.run.validation.structure==='pass';
  const stats=RotationMath.rpmStats(valid?state.rows:[],left,right);
  $('statsScope').textContent=valid?`${fmt(left/1e6,6)}–${fmt(right/1e6,6)} s · ${stats.count} complete raw intervals · flagged intervals included`:'Unavailable for a structurally invalid recording. Inspect raw data in diagnostic view.';
  for(const [id,key]of [['focusMean','mean'],['focusSd','stddev'],['focusVariance','variance'],['focusCv','cv'],['focusMin','min'],['focusMax','max'],['focusMedian','median'],['focusRate','weightedRate']])
    $(id).textContent=fmt(stats[key],2)+(key==='cv'&&stats[key]!==null?'%':'');
}
function renderOverview() {
  const end=focusEnd(),left=Math.max(0,Math.round(state.from*1e6)),right=Number.isFinite(state.to)?Math.min(end,Math.round(state.to*1e6)):end;
  const x=time=>32+(end?time/end:0)*936;const y=index=>78-index/Math.max(1,state.rows.length)*58;
  const rows=plotRows(state.rows);let path='';
  for(const row of rows)path+=`${path?'L':'M'}${x(row.time)},${y(row.index+1)}`;
  $('overview').innerHTML=`<path d="${path}" class="overview-line"/><rect x="${x(left)}" y="10" width="${Math.max(0,x(right)-x(left))}" height="74" class="focus-band" data-drag="move"/><rect x="${x(left)-7}" y="8" width="14" height="78" rx="3" class="focus-handle" data-drag="left"/><rect x="${x(right)-7}" y="8" width="14" height="78" rx="3" class="focus-handle" data-drag="right"/><text x="32" y="102">0 s</text><text x="968" y="102" text-anchor="end">${fmt(end/1e6,3)} s</text>`;
  for(const [id,value]of [['leftBoundary',left],['rightBoundary',right]]) {
    $(id).max=String(end||1);$(id).value=String(value);$(id).disabled=end===0;
    $(id).setAttribute('aria-valuetext',`${fmt(value/1e6,6)} seconds`);
  }
  $('focusSummary').textContent=`${fmt((right-left)/1e6,3)} s window`;
}
function overviewTime(event) {
  const svg=$('overview'),point=svg.createSVGPoint();point.x=event.clientX;point.y=event.clientY;
  const local=point.matrixTransform(svg.getScreenCTM().inverse());
  return Math.round(Math.max(0,Math.min(1,(local.x-32)/936))*focusEnd());
}
let focusDrag=null;
$('overview').addEventListener('pointerdown',event=>{
  if(!state.run||!focusEnd())return;
  const anchor=overviewTime(event);
  let mode=event.target.dataset.drag||'new';
  const left=Math.round(state.from*1e6),right=Number.isFinite(state.to)?Math.round(state.to*1e6):focusEnd();
  if(mode==='move'&&right-left>=focusEnd())mode='new';
  focusDrag={mode,anchor,left,right,pointer:event.pointerId};$('overview').setPointerCapture(event.pointerId);event.preventDefault();
});
$('overview').addEventListener('pointermove',event=>{
  if(!focusDrag||event.pointerId!==focusDrag.pointer)return;
  const at=overviewTime(event);
  let left=focusDrag.left,right=focusDrag.right;
  if(focusDrag.mode==='left')left=Math.min(at,right-1);
  else if(focusDrag.mode==='right')right=Math.max(at,left+1);
  else if(focusDrag.mode==='move')({from:left,to:right}=RotationMath.moveRange(left,right,at-focusDrag.anchor,focusEnd()));
  else {left=Math.min(at,focusDrag.anchor);right=Math.max(at,focusDrag.anchor);}
  setFocus(left,right);
});
function releaseFocus(event){if(focusDrag&&event.pointerId===focusDrag.pointer){focusDrag=null;if($('overview').hasPointerCapture(event.pointerId))$('overview').releasePointerCapture(event.pointerId);}}
$('overview').addEventListener('pointerup',releaseFocus);$('overview').addEventListener('pointercancel',releaseFocus);
$('leftBoundary').addEventListener('input',()=>setFocus(Math.min(Number($('leftBoundary').value),Number($('rightBoundary').value)-1),Number($('rightBoundary').value)));
$('rightBoundary').addEventListener('input',()=>setFocus(Number($('leftBoundary').value),Math.max(Number($('rightBoundary').value),Number($('leftBoundary').value)+1)));
function exportCsv() {
  if(!state.run)return;
  try {settings();}catch(error){$('error').textContent=error.message;return;}
  const vars=settings();
  const quote=value=>'"'+String(value??'').replaceAll('"','""')+'"';
  const textCell=value=>/^[=+@-]/.test(String(value??''))?"'"+value:value;
  const header=['reader_version','display_label','marked','file','sha256','source_crc32','reference_count','reference_crc32','structure','completion','detections_per_revolution','rpm_median_window','interval_ratio','short_interval_ms','range_from_s','range_to_s','raw_event','raw_sequence','elapsed_us','interval_us','raw_rpm','median_rpm','review_flags'];
  const rows=state.rows.filter(r=>r.time/1e6>=state.from&&r.time/1e6<=state.to).map(r=>[READER_VERSION,textCell(state.run.metadata?.label),state.run.metadata?.marked||false,textCell(state.run.name),state.run.sha256,state.run.crc32,state.references?.count,state.references?.crc,state.run.validation.structure,state.run.validation.completion,vars.ppr,vars.window,vars.ratio,vars.floor,state.from,Number.isFinite(state.to)?state.to:'',r.index+1,r.sequence,r.time,r.interval,r.rpm,r.smooth,r.reasons.join(';')].map(quote).join(','));
  const blob=new Blob(['\uFEFF'+header.join(',')+'\r\n'+rows.join('\r\n')],{type:'text/csv;charset=utf-8'});
  const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=state.name.split('/').pop().replace(/\.bin$/i,'')+'-events.csv';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000);
}
$('settings').addEventListener('submit',event=>event.preventDefault());
$('settings').addEventListener('input',()=>{state.page=0;render();});
$('refresh').addEventListener('click',()=>refresh());
$('reset').addEventListener('click',()=>{$('ppr').value='1';$('window').value='1';$('ratio').value='3';$('floor').value='0';state.page=0;render();});
for(const [id,view] of [['countView','count'],['rpmView','rpm']])$(id).addEventListener('click',()=>{state.view=view;for(const [button,value] of [['countView','count'],['rpmView','rpm']]){$(button).classList.toggle('selected',value===view);$(button).setAttribute('aria-pressed',String(value===view));}render();});
$('applyRange').addEventListener('click',applyRange);
$('fullRange').addEventListener('click',()=>setFocus(0,focusEnd()));
$('flaggedOnly').addEventListener('change',()=>{state.page=0;render();});
$('previous').addEventListener('click',()=>{state.page--;renderTable();});$('next').addEventListener('click',()=>{state.page++;renderTable();});
$('checkReference').addEventListener('click',()=>openRun(state.name,true));
$('export').addEventListener('click',exportCsv);
$('showHidden').addEventListener('change',()=>{paintLibrary();const current=visibleRuns().find(r=>r.name===state.name);if(!current){const first=visibleRuns()[0];if(first)openRun(first.name);else clearRun();}else updateIdentity();});
$('renameRun').addEventListener('click',()=>{$('renamePanel').hidden=false;$('labelInput').value=state.run.metadata?.label||'';$('labelInput').focus();});
$('saveLabel').addEventListener('click',()=>saveMetadata({label:$('labelInput').value}));
$('cancelLabel').addEventListener('click',()=>{$('renamePanel').hidden=true;$('renameRun').focus();});
$('markRun').addEventListener('click',()=>saveMetadata({marked:!state.run.metadata?.marked}));
$('hideRun').addEventListener('click',()=>saveMetadata({hidden:!state.run.metadata?.hidden}));
refresh();
