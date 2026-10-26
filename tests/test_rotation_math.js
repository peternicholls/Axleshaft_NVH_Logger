'use strict';
const assert=require('node:assert/strict');
const {rpmStats,boundedRange,moveRange}=require('../analysis-bench/web/rotation-math.js');
const rows=[{time:0,rpm:null,interval:null},{time:100000,rpm:600,interval:100000},{time:300000,rpm:300,interval:200000}];
const stats=rpmStats(rows,0,300000);
assert.equal(stats.count,2);assert.equal(stats.mean,450);assert.equal(stats.median,450);
assert.equal(stats.min,300);assert.equal(stats.max,600);assert.equal(stats.variance,22500);assert.equal(stats.stddev,150);
assert.ok(Math.abs(stats.cv-100/3)<1e-10);assert.equal(stats.weightedRate,400);
const one=rpmStats(rows,100000,300000);assert.equal(one.count,1);assert.equal(one.mean,300);assert.equal(one.variance,0);
assert.equal(rpmStats(rows,150000,300000).count,0); // crossing interval excluded
assert.equal(rpmStats([],0,1).mean,null);assert.equal(rpmStats(rows.slice(0,1),0,1).stddev,null);
const scaled=rpmStats(rows.map(r=>({...r,rpm:r.rpm===null?null:r.rpm/2})),0,300000);
assert.equal(scaled.mean,225);assert.equal(scaled.stddev,75);assert.equal(scaled.variance,5625);assert.equal(scaled.weightedRate,200);
assert.deepEqual(boundedRange(-5,200,100),{from:0,to:100});assert.deepEqual(boundedRange(90,20,100),{from:90,to:91});
assert.deepEqual(boundedRange(0,0,0),{from:0,to:0});
assert.deepEqual(moveRange(20,40,100,100),{from:80,to:100});assert.deepEqual(moveRange(20,40,-100,100),{from:0,to:20});
assert.deepEqual(moveRange(20,40,10,100),{from:30,to:50});
const large=rpmStats(Array.from({length:10000},(_,i)=>({time:i*1000,interval:i?1000:null,rpm:i?1e12+(i%2):null})),0,1e8);
assert.ok(large.variance>0&&large.variance<1); // stable Welford accumulation
console.log('PASS: raw RPM population stats, boundary exclusion, scaling and movable range');
