/* Pure helpers shared by the browser and Node tests. No source data mutation. */
(function(root){
  function rpmStats(rows,fromUs,toUs) {
    let n=0,mean=0,m2=0,min=Infinity,max=-Infinity,totalUs=0;
    const values=[];
    for(let i=1;i<rows.length;i++) {
      const row=rows[i],previous=rows[i-1];
      if(previous.time<fromUs || row.time>toUs || previous.time>toUs || row.time<fromUs || !(row.interval>0) || !Number.isFinite(row.rpm))continue;
      values.push(row.rpm);++n;const delta=row.rpm-mean;mean+=delta/n;m2+=delta*(row.rpm-mean);
      min=Math.min(min,row.rpm);max=Math.max(max,row.rpm);totalUs+=row.interval;
    }
    if(!n)return {count:0,mean:null,median:null,min:null,max:null,variance:null,stddev:null,cv:null,weightedRate:null};
    values.sort((a,b)=>a-b);
    const variance=Math.max(0,m2/n),stddev=Math.sqrt(variance);
    // One interval's RPM*duration gives the same detections/revolution factor.
    const revolutions=rows.find(r=>Number.isFinite(r.rpm)&&r.interval>0);
    const factor=revolutions ? revolutions.rpm*revolutions.interval : null;
    return {count:n,mean,median:(values[Math.floor((n-1)/2)]+values[Math.floor(n/2)])/2,min,max,
      variance,stddev,cv:mean>0?stddev/mean*100:null,weightedRate:factor===null?null:factor*n/totalUs};
  }
  function boundedRange(from,to,end) {
    end=Math.max(0,Math.round(end));
    if(!end)return {from:0,to:0};
    from=Math.max(0,Math.min(end-1,Math.round(from)));
    to=Math.max(from+1,Math.min(end,Math.round(to)));
    return {from,to};
  }
  function moveRange(from,to,delta,end) {
    const current=boundedRange(from,to,end),width=current.to-current.from;
    const left=Math.max(0,Math.min(end-width,Math.round(current.from+delta)));
    return {from:left,to:left+width};
  }
  const api={rpmStats,boundedRange,moveRange};
  if(typeof module!=='undefined'&&module.exports)module.exports=api;else root.RotationMath=api;
})(globalThis);
